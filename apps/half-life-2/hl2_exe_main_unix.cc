// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// The entry point for *nix Half-Life 2 process.

#include <filesystem>

#include "app_version_config.h"
#include "base/deps/g3log/scoped_g3log_initializer.h"
#include "base/deps/sdl/message_box.h"
#include "base/unique_module_ptr.h"
#include "bootmgr/bootmgr_main.h"
#include "build/static_settings_config.h"

namespace {
/**
 * @brief Load and run bootmgr.
 * @param argc App arguments count.
 * @param argv App arguments.
 * @return App exit code.
 */
int BootmgrStartup(int argc, char** argv) noexcept {
  using namespace wb::base;

  std::error_code rc;
  auto app_path = std::filesystem::current_path(rc);
  if (rc) {
    wb::sdl::Fatal(WB_PRODUCT_FILE_DESCRIPTION_STRING, rc)
        << "Can't get current directory.  Unable to load the app.";
  }

  app_path /= "libbootmgr.so." WB_PRODUCT_VERSION_INFO_STRING;

  const std::string boot_manager_path{app_path.string()};
  const int boot_manager_load_flags{RTLD_LAZY | RTLD_LOCAL};

  const auto boot_manager_load_result = unique_module_ptr::FromLibraryOnPath(
      boot_manager_path, boot_manager_load_flags);
  if (const auto* boot_manager_module =
          std_ext::GetSuccessResult(boot_manager_load_result)) {
    using BootmgrMainFunction = decltype(&BootmgrMain);

    constexpr char kBootManagerMainFunctionName[]{"BootmgrMain"};

    // Good, try to find and launch bootmgr.
    const auto boot_manager_entry_result =
        boot_manager_module->GetAddressAs<BootmgrMainFunction>(
            kBootManagerMainFunctionName);
    if (const auto* boot_manager_main =
            std_ext::GetSuccessResult(boot_manager_entry_result)) {
      return (*boot_manager_main)(
          {WB_PRODUCT_FILE_DESCRIPTION_STRING, argv, argc});
    }

    rc = std::get<std::error_code>(boot_manager_entry_result);
    wb::sdl::Fatal(WB_PRODUCT_FILE_DESCRIPTION_STRING, rc)
        << "Can't get '" << kBootManagerMainFunctionName
        << "' entry point from '" << boot_manager_path
        << "'.  Looks like app is broken, please, reinstall the one.";
  } else {
    rc = std::get<std::error_code>(boot_manager_load_result);
    wb::sdl::Fatal(WB_PRODUCT_FILE_DESCRIPTION_STRING, rc)
        << "Can't load boot manager '" << boot_manager_path
        << "'.  Please, reinstall the app.";
  }

  return rc.value();
}
}  // namespace

int main(int argc, char* argv[]) {
  using namespace wb::base;

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
  const deps::g3log::ScopedG3LogInitializer scoped_g3log_initializer{
      argv[0], wb::build::settings::kPathToMainLogFile};

  return BootmgrStartup(argc, argv);
}