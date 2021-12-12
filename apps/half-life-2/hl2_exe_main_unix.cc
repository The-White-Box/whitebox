// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// The entry point for *nix Half-Life 2 process.

#include "app_version_config.h"
#include "apps/i18n_creator.h"
#include "base/deps/abseil/flags/flag.h"
#include "base/deps/abseil/flags/parse.h"
#include "base/deps/abseil/flags/usage.h"
#include "base/deps/abseil/flags/usage_config.h"
#include "base/deps/abseil/strings/str_cat.h"
#include "base/deps/fmt/core.h"
#include "base/deps/g3log/scoped_g3log_initializer.h"
#include "base/intl/l18n.h"
#include "base/intl/scoped_process_locale.h"
#include "base/scoped_shared_library.h"
#include "base/std2/filesystem_ext.h"
#include "build/static_settings_config.h"
#include "hl2_exe_flags.h"
#include "whitebox-boot-manager/main.h"
#include "whitebox-ui/fatal_dialog.h"

namespace {

/**
 * @brief Makes program version string.
 * @param program_path Full path to program.
 * @param version Program version.
 * @return version string.
 */
[[nodiscard]] std::string VersionString(
    const std::filesystem::path& program_path, std::string_view version) {
  return absl::StrCat(program_path.filename().string(), " version ", version
#ifndef NDEBUG
                      ,
                      " (Debug Build)\n"
#endif
  );
}

/**
 * @brief Parses command line flags.
 * @param argc App arguments count.
 * @param argv App arguments.
 * @return Unparsed positional flags.
 */
[[nodiscard]] std::vector<char*> ParseCommandLine(int argc,
                                                  char** argv) noexcept {
  const absl::FlagsUsageConfig flags_usage_config = {
      .contains_helpshort_flags = {},
      .contains_help_flags = {},
      .contains_helppackage_flags = {},
      .version_string =
          [path = argv[0]] {
            return VersionString(std::filesystem::path{path},
                                 WB_PRODUCT_VERSION_INFO_STRING);
          },
      .normalize_filename = {}};
  // Set custom version message as we need more info.
  absl::SetFlagsUsageConfig(flags_usage_config);
  // Command line flags should be early initialized, but after logging (depends
  // on it).
  absl::SetProgramUsageMessage(
      absl::StrCat(wb::apps::half_life_2::kUsageMessage, argv[0]));
  std::vector<char*> positional_flags{absl::ParseCommandLine(argc, argv)};

  std::string command_line;
  for (int i{0}; i < argc; ++i) {
    absl::StrAppend(&command_line, argv[i]);
    if (i != argc - 1) {
      absl::StrAppend(&command_line, " ");
    }
  }

  G3LOG(INFO) << WB_PRODUCT_FILE_DESCRIPTION_STRING " started as "
              << command_line;

  return positional_flags;
}

/**
 * @brief Load and run boot manager.
 * @param argc App arguments count.
 * @param argv App arguments.
 * @return App exit code.
 */
int BootmgrStartup(int argc, char** argv) noexcept {
  using namespace wb::base;
  using namespace wb::ui;

  // Setup command line flags as they are used early.
  std::vector<char*> positional_flags{ParseCommandLine(argc, argv)};

  // Start with specifying UTF-8 locale for all user-facing data.
  const intl::ScopedProcessLocale scoped_process_locale{
      intl::ScopedProcessLocaleCategory::kAll, intl::locales::kUtf8Locale};
  const auto intl = wb::apps::CreateIntl(WB_PRODUCT_FILE_DESCRIPTION_STRING,
                                         scoped_process_locale);

  // Get not current directory, but directory from which exe is launched.
  // Prevents DLL / SO planting attacks.
  auto app_path_result = std2::filesystem::get_executable_directory();
  if (const auto* rc = std2::get_error(app_path_result)) WB_ATTRIBUTE_UNLIKELY {
      wb::ui::FatalDialog(
          intl::l18n_fmt(intl, "{0} - Error",
                         WB_PRODUCT_FILE_DESCRIPTION_STRING),
          *rc,
          intl::l18n(intl,
                     "Please, check app is installed correctly and you have "
                     "enough permissions to run it."),
          wb::ui::FatalDialogContext{intl.Layout()},
          intl::l18n(intl,
                     "Can't get current directory.  May be app located too "
                     "deep (> 1024)?"));
    }

  auto app_path = *std2::get_result(app_path_result);
  app_path /= "libwhitebox-boot-manager.so." WB_PRODUCT_VERSION_INFO_STRING;

  const std::string boot_manager_path{app_path.string()};
  const auto boot_manager_library = ScopedSharedLibrary::FromLibraryOnPath(
      boot_manager_path, RTLD_LAZY | RTLD_LOCAL);
  if (const auto* boot_manager = std2::get_result(boot_manager_library))
    WB_ATTRIBUTE_LIKELY {
      using BootManagerMain = decltype(&BootmgrMain);
      // NOLINTNEXTLINE(modernize-avoid-c-arrays)
      constexpr char kBootManagerMainName[]{"BootmgrMain"};

      // Good, try to find and launch boot manager.
      const auto boot_manager_entry =
          boot_manager->GetAddressAs<BootManagerMain>(kBootManagerMainName);
      if (const auto* boot_manager_main = std2::get_result(boot_manager_entry))
        WB_ATTRIBUTE_LIKELY {
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
          return (*boot_manager_main)(
              {WB_PRODUCT_FILE_DESCRIPTION_STRING, command_line_flags, intl});
        }

      FatalDialog(
          intl::l18n_fmt(intl, "{0} - Error",
                         WB_PRODUCT_FILE_DESCRIPTION_STRING),
          std::get<std::error_code>(boot_manager_entry),
          intl::l18n(intl,
                     "Please, check app is installed correctly and you have "
                     "enough permissions to run it."),
          FatalDialogContext{intl.Layout()},
          intl::l18n_fmt(intl, "Can't get '{0}' entry point from '{1}'.",
                         kBootManagerMainName, boot_manager_path));
    }
  else {
    FatalDialog(
        intl::l18n_fmt(intl, "{0} - Error", WB_PRODUCT_FILE_DESCRIPTION_STRING),
        std::get<std::error_code>(boot_manager_library),
        intl::l18n(intl,
                   "Please, check app is installed correctly and you have "
                   "enough permissions to run it."),
        FatalDialogContext{intl.Layout()},
        intl::l18n_fmt(intl, "Can't load boot manager '{0}'.",
                       boot_manager_path));
  }
}

}  // namespace

int main(int argc, char* argv[]) {
  // Initialize g3log logging library first as logs are used extensively.
  const wb::base::deps::g3log::ScopedG3LogInitializer scoped_g3log_initializer{
      argv[0], wb::build::settings::kPathToMainLogFile};

  return BootmgrStartup(argc, argv);
}