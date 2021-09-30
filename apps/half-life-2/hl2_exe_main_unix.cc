// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// The entry point for *nix Half-Life 2 process.

#include "app_version_config.h"
#include "base/deps/fmt/core.h"
#include "base/deps/g3log/scoped_g3log_initializer.h"
#include "base/intl/scoped_process_locale.h"
#include "base/scoped_shared_library.h"
#include "base/std2/filesystem_ext.h"
#include "build/static_settings_config.h"
#include "whitebox-boot-manager/boot_manager_main.h"
#include "whitebox-ui/fatal_dialog.h"

namespace {
/**
 * @brief Creates internationalization lookup.
 * @param user_locale User locale.
 * @return Internationalization lookup.
 */
[[nodiscard]] wb::base::intl::LookupWithFallback CreateIntl(
    const std::string& user_locale) noexcept {
  auto intl_lookup_result{
      wb::base::intl::LookupWithFallback::New({user_locale})};
  auto intl_lookup =
      std::get_if<wb::base::intl::LookupWithFallback>(&intl_lookup_result);

  G3LOG_IF(FATAL, !intl_lookup)
      << "Unable to create localization strings lookup for locale "
      << user_locale << ".";
  return std::move(*intl_lookup);
}

/**
 * @brief Load and run bootmgr.
 * @param argc App arguments count.
 * @param argv App arguments.
 * @param intl Localization service.
 * @return App exit code.
 */
int BootmgrStartup(int argc, char** argv) noexcept {
  using namespace wb::base;

  // Start with specifying UTF-8 locale for all user-facing data.
  const intl::ScopedProcessLocale scoped_process_locale{
      intl::ScopedProcessLocaleCategory::kAll, intl::locales::kUtf8Locale};
  const std::string user_locale{
      scoped_process_locale.GetCurrentLocale().value_or(
          intl::locales::kFallbackLocale)};
  G3LOG(INFO) << WB_PRODUCT_FILE_DESCRIPTION_STRING << " using " << user_locale
              << " locale for UI.";

  auto intl = CreateIntl(user_locale);

  // Get not current directory, but directory from which exe is launched.
  // Prevents DLL / SO planting attacks.
  std::error_code rc;
  auto app_path = std2::GetExecutableDirectory(rc);
  if (rc) WB_ATTRIBUTE_UNLIKELY {
      wb::ui::FatalDialog(
          intl::l18n_fmt(intl, "{0} - Error",
                         WB_PRODUCT_FILE_DESCRIPTION_STRING),
          rc,
          intl::l18n(intl,
                     "Please, check app is installed correctly and you have "
                     "enough permissions to run it."),
          {intl.Layout()} intl::l18n(intl,
                                     "Can't get current directory.  May be app "
                                     "located too deep (> 1024)?"),
          rc);
    }

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
          return (*boot_manager_main)(
              {WB_PRODUCT_FILE_DESCRIPTION_STRING, argv, argc, intl});
        }

      wb::ui::FatalDialog(
          intl::l18n_fmt(intl, "{0} - Error",
                         WB_PRODUCT_FILE_DESCRIPTION_STRING),
          std::get<std::error_code>(boot_manager_entry),
          intl::l18n(intl,
                     "Please, check app is installed correctly and you have "
                     "enough permissions to run it."),
          {intl.Layout()},
          intl::l18n_fmt(intl, "Can't get '{0}' entry point from '{1}'.",
                         kBootManagerMainName, boot_manager_path));
    }
  else {
    wb::ui::FatalDialog(
        intl::l18n_fmt(intl, "{0} - Error", WB_PRODUCT_FILE_DESCRIPTION_STRING),
        std::get<std::error_code>(boot_manager_library),
        intl::l18n(intl,
                   "Please, check app is installed correctly and you have "
                   "enough permissions to run it."),
        {intl.Layout()},
        intl::l18n_fmt(intl, "Can't load boot manager '{0}'.",
                       boot_manager_path));
  }
}
}  // namespace

int main(int argc, char* argv[]) {
#ifndef NDEBUG
  // TODO(dimhotepus): Add signed only file support.
  // Simplifies debugging experience, no need to sign targets.
  // std::string debug_command_line{command_line};
  // debug_command_line.append(" ");
  // debug_command_line.append(
  //     wb::base::switches::insecure::kAllowUnsignedModuleTargetFlag);
  //
  // command_line = debug_command_line.data();
#endif

  // Initialize g3log logging library first as logs are used extensively.
  const wb::base::deps::g3log::ScopedG3LogInitializer scoped_g3log_initializer{
      argv[0], wb::build::settings::kPathToMainLogFile};

  return BootmgrStartup(argc, argv);
}