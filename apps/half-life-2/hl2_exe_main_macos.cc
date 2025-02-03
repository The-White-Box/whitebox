// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// The entry point for Mac Half-Life 2 process.
//
// Based on
// https://github.com/chromium/chromium/blob/master/chrome/app/chrome_exe_main_mac.cc

#include <dlfcn.h>
#include <libgen.h>
#include <mach-o/dyld.h>
#include <unistd.h>

#include <cerrno>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string_view>
#include <system_error>

#include "app_version_config.h"
#include "apps/boot_heap_allocator.h"
#include "apps/cpu_feature_checks.h"
#include "apps/i18n_creator.h"
#include "apps/parse_command_line.h"
#include "base/default_new_handler.h"
#include "base/deps/abseil/flags/flag.h"
#include "base/deps/abseil/strings/str_join.h"
#include "base/deps/g3log/g3log.h"
#include "base/deps/g3log/scoped_g3log_initializer.h"
#include "base/deps/sdl/message_box.h"
#include "base/intl/l18n.h"
#include "base/intl/scoped_process_locale.h"
#include "base/scoped_new_handler.h"
#include "base/scoped_shared_library.h"
#include "base/std2/system_error_ext.h"
#include "boot-manager/main.h"
#include "build/static_settings_config.h"
#include "hl2_exe_flags.h"
#include "ui/fatal_dialog.h"

#ifdef WB_MI_MALLOC
#include "base/deps/mimalloc/scoped_dump_mimalloc_main_stats.h"
#endif

__attribute__((visibility("default"))) int main(int argc, char* argv[]) {
  using namespace wb::base;
  using namespace wb::ui;

  // Initialize g3log logging library first as logs are used extensively.
  const deps::g3log::ScopedG3LogInitializer scoped_g3log_initializer{
      argv[0], wb::build::settings::kPathToMainLogFile};

  // Install heap allocator tracing / set options.
  wb::apps::BootHeapAllocator();

  // Setup command line flags as they are used early.
  std::vector<char*> positional_flags{wb::apps::ParseCommandLine(
      argc, argv,
      {.app_name = WB_PRODUCT_FILE_DESCRIPTION_STRING,
       .app_version = WB_PRODUCT_FILE_VERSION_INFO_STRING,
       .app_usage = wb::apps::half_life_2::kUsageMessage})};

  // Start with specifying UTF-8 locale for all user-facing data.
  const intl::ScopedProcessLocale scoped_process_locale{
      intl::ScopedProcessLocaleCategory::kAll, intl::locales::kUtf8Locale};
  const auto l18n = wb::apps::CreateIntl(WB_PRODUCT_FILE_DESCRIPTION_STRING,
                                         scoped_process_locale);

  // Query CPU support for required features.  In case any required feature is
  // missed we return all required features with support state.
  const std::vector<wb::apps::CpuFeature> cpu_features_support{
      wb::apps::QueryRequiredCpuFeatures()};
  if (!cpu_features_support.empty()) [[unlikely]] {
    const std::string cpu_features_support_state{absl::StrJoin(
        cpu_features_support, "\n",
        [&](std::string* out, const wb::apps::CpuFeature& feature_support) {
          absl::StrAppend(
              out, intl::l18n_fmt(l18n, "{0}     {1}", feature_support.name,
                                  feature_support.is_supported ? "✓" : "❌"));
        })};
    return wb::ui::FatalDialog(
        intl::l18n_fmt(l18n, "{0} - Error", WB_PRODUCT_FILE_DESCRIPTION_STRING),
        std2::system_last_error_code(ENODEV),
        intl::l18n(l18n,
                   "Sorry, your CPU has missed some required features to run "
                   "the game."),
        FatalDialogContext{l18n.Layout()},
        intl::l18n_fmt(l18n, "CPU features support table for {0}:\n{1}",
                       wb::apps::QueryCpuBrand(), cpu_features_support_state));
  }

  uint32_t exec_path_size{0};
  int rv{_NSGetExecutablePath(nullptr, &exec_path_size)};
  if (rv != -1) [[unlikely]] {
    return wb::ui::FatalDialog(
        intl::l18n_fmt(l18n, "{0} - Error", WB_PRODUCT_FILE_DESCRIPTION_STRING),
        std2::system_last_error_code(rv),
        intl::l18n(l18n,
                   "Sorry, unable to load the app.  Please, contact support."),
        FatalDialogContext{l18n.Layout()},
        "_NSGetExecutablePath: get length failed.  Unable to load the app.");
  }

  std::unique_ptr<char[]> exec_path{std::make_unique<char[]>(exec_path_size)};
  rv = _NSGetExecutablePath(exec_path.get(), &exec_path_size);
  if (rv != 0) [[unlikely]] {
    return wb::ui::FatalDialog(
        intl::l18n_fmt(l18n, "{0} - Error", WB_PRODUCT_FILE_DESCRIPTION_STRING),
        std2::system_last_error_code(rv),
        intl::l18n(l18n,
                   "Sorry, unable to load the app.  Please, contact support."),
        FatalDialogContext{l18n.Layout()},
        "_NSGetExecutablePath: get path failed.  Unable to load the app.");
  }

  constexpr char rel_path[]{
      "../Frameworks/" WB_PRODUCT_NAME_STRING
      " Framework.framework/Versions/" WB_PRODUCT_FILE_VERSION_INFO_STRING
      "/" WB_PRODUCT_FILE_DESCRIPTION_STRING " Framework"};

  // Slice off the last part of the main executable path, and append the
  // version framework information.
  const char* parent_dir{dirname(exec_path.get())};
  if (!parent_dir) [[unlikely]] {
    return wb::ui::FatalDialog(
        intl::l18n_fmt(l18n, "{0} - Error", WB_PRODUCT_FILE_DESCRIPTION_STRING),
        std2::posix_last_error_code(errno),
        intl::l18n_fmt(
            l18n,
            "Unable to get parent directory for '{0}'.  Please, contact "
            "support.",
            exec_path.get()),
        FatalDialogContext{l18n.Layout()},
        intl::l18n_fmt(l18n, "dirname '{0}' failed.", exec_path.get()));
  }

  const size_t parent_dir_len{strlen(parent_dir)};
  const size_t rel_path_len{strlen(rel_path)};
  // 2 accounts for a trailing NUL byte and the '/' in the middle of the paths.
  const size_t framework_path_size{parent_dir_len + rel_path_len + 2};
  std::unique_ptr<char[]> framework_path{
      std::make_unique<char[]>(framework_path_size)};
  snprintf(framework_path.get(), framework_path_size, "%s/%s/%s", parent_dir,
           rel_path,
           "libwhitebox-boot-manager." WB_PRODUCT_VERSION_INFO_STRING ".dylib");

  const auto boot_manager_library = ScopedSharedLibrary::FromLibraryOnPath(
      framework_path.get(), RTLD_LAZY | RTLD_LOCAL | RTLD_FIRST);
  if (!boot_manager_library.has_value()) [[unlikely]] {
    return wb::ui::FatalDialog(
        intl::l18n_fmt(l18n, "{0} - Error", WB_PRODUCT_FILE_DESCRIPTION_STRING),
        boot_manager_library.error(),
        intl::l18n(l18n,
                   "Please, check app is installed correctly and you have "
                   "enough permissions to run it."),
        FatalDialogContext{l18n.Layout()},
        intl::l18n_fmt(l18n, "Can't load boot manager '{0}'.",
                       framework_path.get()));
  }

  using BootManagerMain = decltype(&BootManagerMain);
  // NOLINTNEXTLINE(modernize-avoid-c-arrays)
  constexpr char kBootManagerMainName[]{"BootManagerMain"};

  const wb::base::ScopedSharedLibrary& boot_manager = *boot_manager_library;

  // Good, try to find and launch boot manager.
  const auto boot_manager_entry =
      boot_manager.GetAddressAs<BootManagerMain>(kBootManagerMainName);
  if (!boot_manager_entry.has_value()) [[unlikely]] {
    return FatalDialog(
        intl::l18n_fmt(l18n, "{0} - Error", WB_PRODUCT_FILE_DESCRIPTION_STRING),
        boot_manager_entry.error(),
        intl::l18n(l18n,
                   "Please, check app is installed correctly and you have "
                   "enough permissions to run it."),
        FatalDialogContext{l18n.Layout()},
        intl::l18n_fmt(l18n, "Can't get '{0}' entry point from '{1}'.",
                       kBootManagerMainName, framework_path.get()));
  }

  wb::apps::flags::AssetsPath assets_path{absl::GetFlag(FLAGS_assets_path)};

  const std::uint32_t attempts_to_retry_allocate_memory{
      absl::GetFlag(FLAGS_attempts_to_retry_allocate_memory)};
  const wb::apps::flags::WindowWidth main_window_width{
      absl::GetFlag(FLAGS_main_window_width)};
  const wb::apps::flags::WindowHeight main_window_height{
      absl::GetFlag(FLAGS_main_window_height)};
  const bool should_dump_heap_allocator_statistics_on_exit{
      absl::GetFlag(FLAGS_should_dump_heap_allocator_statistics_on_exit)};
  const wb::boot_manager::CommandLineFlags command_line_flags{
      .positional_flags = std::move(positional_flags),
      .assets_path = std::move(assets_path.value),
      .attempts_to_retry_allocate_memory = attempts_to_retry_allocate_memory,
      .main_window_width = main_window_width.size,
      .main_window_height = main_window_height.size,
      .insecure_allow_unsigned_module_target = false,
      .should_dump_heap_allocator_statistics_on_exit =
          should_dump_heap_allocator_statistics_on_exit};

#ifdef WB_MI_MALLOC
  // Dumps mimalloc stats on exit?
  const wb::mi::ScopedDumpMiMainStats scoped_dump_mi_main_stats{
      should_dump_heap_allocator_statistics_on_exit};
#endif  // WB_MI_MALLOC

  // Handle new allocation failure.
  ScopedNewHandler scoped_new_handler{DefaultNewFailureHandler,
                                      attempts_to_retry_allocate_memory};
  // Set it as global handler.  C++ API is too strict here and we can't pass
  // state into void(void), so need global variable to access state in
  // handler.
  InstallGlobalScopedNewHandler(std::move(scoped_new_handler));

  const auto boot_manager_main = *boot_manager_entry;
  G3CHECK(!!boot_manager_main);

  rv = boot_manager_main(std::string_view{WB_PRODUCT_FILE_DESCRIPTION_STRING},
                         command_line_flags, l18n);

  // exit, don't return from main, to avoid the apparent removal of main
  // from stack backtraces under tail call optimization.
  exit(rv);
}