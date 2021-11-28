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
#include <system_error>

#include "app_version_config.h"
#include "base/deps/abseil/flags/flag.h"
#include "base/deps/abseil/flags/parse.h"
#include "base/deps/abseil/flags/usage.h"
#include "base/deps/abseil/strings/str_cat.h"
#include "base/deps/g3log/g3log.h"
#include "base/deps/g3log/scoped_g3log_initializer.h"
#include "base/deps/sdl/message_box.h"
#include "base/intl/l18n.h"
#include "base/intl/scoped_process_locale.h"
#include "base/scoped_shared_library.h"
#include "build/static_settings_config.h"
#include "hl2_exe_flags.h"
#include "whitebox-boot-manager/main.h"

namespace {

/**
 * @brief Creates internationalization lookup.
 * @param scoped_process_locale Process locale.
 * @return Internationalization lookup.
 */
[[nodiscard]] wb::base::intl::LookupWithFallback CreateIntl(
    const wb::base::intl::ScopedProcessLocale& scoped_process_locale) noexcept {
  using namespace wb::base::intl;

  const std::optional<std::string> maybe_user_locale{
      scoped_process_locale.GetCurrentLocale()};
  G3LOG_IF(WARNING, !maybe_user_locale.has_value())
      << WB_PRODUCT_FILE_DESCRIPTION_STRING << " unable to use UTF8 locale '"
      << locales::kUtf8Locale << "' for UI, fallback to '"
      << locales::kFallbackLocale << "'.";

  const std::string user_locale{
      maybe_user_locale.value_or(locales::kFallbackLocale)};
  G3LOG(INFO) << WB_PRODUCT_FILE_DESCRIPTION_STRING << " using " << user_locale
              << " locale for UI.";

  auto intl_lookup_result{LookupWithFallback::New({user_locale})};
  auto intl_lookup = std::get_if<LookupWithFallback>(&intl_lookup_result);

  G3LOG_IF(FATAL, !intl_lookup)
      << "Unable to create localization strings lookup for locale "
      << user_locale << ".";
  return std::move(*intl_lookup);
}

}  // namespace

__attribute__((visibility("default"))) int main(int argc, char* argv[]) {
  // Initialize g3log logging library first as logs are used extensively.
  const wb::base::deps::g3log::ScopedG3LogInitializer scoped_g3log_initializer{
      argv[0], wb::build::settings::kPathToMainLogFile};

  absl::SetProgramUsageMessage(
      absl::StrCat(wb::apps::half_life_2::kUsageMessage, argv[0]));
  std::vector<char*> positional_flags{absl::ParseCommandLine(argc, argv)};

  // Start with specifying UTF-8 locale for all user-facing data.
  const intl::ScopedProcessLocale scoped_process_locale{
      intl::ScopedProcessLocaleCategory::kAll, intl::locales::kUtf8Locale};
  const auto intl = CreateIntl(scoped_process_locale);

  uint32_t exec_path_size{0};
  int rv{_NSGetExecutablePath(nullptr, &exec_path_size)};
  if (rv != -1) WB_ATTRIBUTE_UNLIKELY {
      wb::sdl::Fatal(WB_PRODUCT_FILE_DESCRIPTION_STRING, std::error_code{rv})
          << "_NSGetExecutablePath: get length failed.  Unable to load the "
             "app.";
    }

  std::unique_ptr<char[]> exec_path{std::make_unique<char[]>(exec_path_size)};
  rv = _NSGetExecutablePath(exec_path.get(), &exec_path_size);
  if (rv != 0) WB_ATTRIBUTE_UNLIKELY {
      wb::sdl::Fatal(WB_PRODUCT_FILE_DESCRIPTION_STRING, std::error_code{rv})
          << "_NSGetExecutablePath: get path failed.  Unable to load the app.";
    }

  constexpr char rel_path[]{
      "../Frameworks/" WB_PRODUCT_NAME_STRING
      " Framework.framework/Versions/" WB_PRODUCT_FILE_VERSION_INFO_STRING
      "/" WB_PRODUCT_FILE_DESCRIPTION_STRING " Framework"};

  // Slice off the last part of the main executable path, and append the
  // version framework information.
  const char* parent_dir{dirname(exec_path.get())};
  if (!parent_dir) WB_ATTRIBUTE_UNLIKELY {
      wb::sdl::Fatal(WB_PRODUCT_FILE_DESCRIPTION_STRING, std::error_code{errno})
          << "dirname '" << exec_path.get() << "'.";
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

  using namespace wb::base;

  const auto boot_manager_load_result = ScopedSharedLibrary::FromLibraryOnPath(
      framework_path.get(), RTLD_LAZY | RTLD_LOCAL | RTLD_FIRST);
  if (const auto* rc = std2::get_error(boot_manager_load_result))
    WB_ATTRIBUTE_UNLIKELY {
      wb::sdl::Fatal(WB_PRODUCT_FILE_DESCRIPTION_STRING, *rc)
          << "Can't load boot manager '" << framework_path.get() << ".";
    }

  const auto boot_manager_module =
      std::get<ScopedSharedLibrary>(boot_manager_load_result);
  constexpr char kBootManagerMainFunctionName[]{"BootmgrMain"};

  using BootManagerMainFunction = decltype(&BootmgrMain);

  // Good, try to find and launch boot manager.
  const auto boot_manager_entry_result =
      boot_manager_module->GetAddressAs<BootManagerMainFunction>(
          kBootmgrMainFunctionName);
  if (const auto* rc = std2::get_error(bootmgr_entry_result))
    WB_ATTRIBUTE_UNLIKELY {
      wb::sdl::Fatal(WB_PRODUCT_FILE_DESCRIPTION_STRING, *rc)
          << "Can't get '" << kBootmgrMainFunctionName << "' entry point from '"
          << framework_path.get() << "'.";
    }

  const auto boot_manager_main =
      std::get<BootManagerMainFunction>(bootmgr_entry_result);

  const wb::apps::half_life_2::WindowWidth main_window_width{
      absl::GetFlag(FLAGS_main_window_width)};
  const wb::apps::half_life_2::WindowHeight main_window_height{
      absl::GetFlag(FLAGS_main_window_height)};
  const wb::boot_manager::CommandLineFlags command_line_flags{
      .positional_flags = std::move(positional_flags),
      .main_window_width = main_window_width.size,
      .main_window_height = main_window_height.size,
      .insecure_allow_unsigned_module_target = false,
  };

  rv = boot_manager_main(WB_PRODUCT_FILE_DESCRIPTION_STRING, command_line_flags,
                         intl);

  // exit, don't return from main, to avoid the apparent removal of main
  // from stack backtraces under tail call optimization.
  exit(rv);
}