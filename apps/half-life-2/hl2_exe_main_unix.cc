// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// The entry point for *nix Half-Life 2 process.

#include "app_version_config.h"
#include "apps/boot_heap_allocator.h"
#include "apps/cpu_feature_checks.h"
#include "apps/i18n_creator.h"
#include "apps/parse_command_line.h"
#include "base/default_new_handler.h"
#include "base/deps/abseil/flags/flag.h"
#include "base/deps/abseil/strings/str_join.h"
#include "base/deps/fmt/core.h"
#include "base/deps/g3log/scoped_g3log_initializer.h"
#include "base/intl/l18n.h"
#include "base/intl/scoped_process_locale.h"
#include "base/scoped_new_handler.h"
#include "base/scoped_shared_library.h"
#include "base/std2/filesystem_ext.h"
#include "boot-manager/main.h"
#include "build/static_settings_config.h"
#include "hl2_exe_flags.h"
#include "ui/fatal_dialog.h"

#ifdef WB_MI_MALLOC
#include "base/deps/mimalloc/scoped_dump_mimalloc_main_stats.h"
#endif

namespace {

/**
 * @brief Load and run boot manager.
 * @param argc App arguments count.
 * @param argv App arguments.
 * @return App exit code.
 */
int BootManagerStartup(int argc, char** argv) noexcept {
  using namespace wb::base;
  using namespace wb::ui;

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
                                  feature_support.is_supported ? "✅" : "❌"));
        })};
    return wb::ui::FatalDialog(
        intl::l18n_fmt(l18n, "{0} - Error", WB_PRODUCT_FILE_DESCRIPTION_STRING),
        std2::system_last_error_code(ENODEV),
        intl::l18n(l18n,
                   "Sorry, your CPU has missed some required features to run "
                   "the game."),
        wb::ui::FatalDialogContext{l18n.Layout()},
        intl::l18n_fmt(l18n, "CPU features support table for {0}:\n{1}",
                       wb::apps::QueryCpuBrand(), cpu_features_support_state));
  }

  // Get not current directory, but directory from which exe is launched.
  // Prevents DLL / SO planting attacks.
  auto app_path_result = std2::filesystem::get_executable_directory();
  if (const auto* rc = std2::get_error(app_path_result)) [[unlikely]] {
    return wb::ui::FatalDialog(
        intl::l18n_fmt(l18n, "{0} - Error", WB_PRODUCT_FILE_DESCRIPTION_STRING),
        *rc,
        intl::l18n(l18n,
                   "Please, check app is installed correctly and you have "
                   "enough permissions to run it."),
        wb::ui::FatalDialogContext{l18n.Layout()},
        intl::l18n(l18n,
                   "Can't get current directory.  May be app located too "
                   "deep (> 1024)?"));
  }

  auto app_path = *std2::get_result(app_path_result);
  app_path /= "libwhitebox-boot-manager.so." WB_PRODUCT_VERSION_INFO_STRING;

  const std::string boot_manager_path{app_path.string()};
  const auto boot_manager_library = ScopedSharedLibrary::FromLibraryOnPath(
      boot_manager_path, RTLD_LAZY | RTLD_LOCAL);
  if (const auto* boot_manager = std2::get_result(boot_manager_library))
      [[likely]] {
    using BootManagerMain = decltype(&BootManagerMain);
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    constexpr char kBootManagerMainName[]{"BootManagerMain"};

    // Good, try to find and launch boot manager.
    const auto boot_manager_entry =
        boot_manager->GetAddressAs<BootManagerMain>(kBootManagerMainName);
    if (const auto* boot_manager_main = std2::get_result(boot_manager_entry))
        [[likely]] {
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
          .attempts_to_retry_allocate_memory =
              attempts_to_retry_allocate_memory,
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
      // Set it as global handler.  C++ API is too strict here, and we can't
      // pass state into void(void), so need global variable to access state
      // in handler.
      InstallGlobalScopedNewHandler(std::move(scoped_new_handler));

      return (*boot_manager_main)(
          {WB_PRODUCT_FILE_DESCRIPTION_STRING, command_line_flags, l18n});
    }

    return FatalDialog(
        intl::l18n_fmt(l18n, "{0} - Error", WB_PRODUCT_FILE_DESCRIPTION_STRING),
        std::get<std::error_code>(boot_manager_entry),
        intl::l18n(l18n,
                   "Please, check app is installed correctly and you have "
                   "enough permissions to run it."),
        FatalDialogContext{l18n.Layout()},
        intl::l18n_fmt(l18n, "Can't get '{0}' entry point from '{1}'.",
                       kBootManagerMainName, boot_manager_path));
  }

  return FatalDialog(
      intl::l18n_fmt(l18n, "{0} - Error", WB_PRODUCT_FILE_DESCRIPTION_STRING),
      std::get<std::error_code>(boot_manager_library),
      intl::l18n(l18n,
                 "Please, check app is installed correctly and you have "
                 "enough permissions to run it."),
      FatalDialogContext{l18n.Layout()},
      intl::l18n_fmt(l18n, "Can't load boot manager '{0}'.",
                     boot_manager_path));
}

}  // namespace

int main(int argc, char* argv[]) {
  // Initialize g3log logging library first as logs are used extensively.
  const wb::base::deps::g3log::ScopedG3LogInitializer scoped_g3log_initializer{
      argv[0], wb::build::settings::kPathToMainLogFile};

  // Install heap allocator tracing / set options.
  wb::apps::BootHeapAllocator();

  return BootManagerStartup(argc, argv);
}