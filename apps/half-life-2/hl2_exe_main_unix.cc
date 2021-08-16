// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// The entry point for *nix Half-Life 2 process.

#include <filesystem>

#include "app_version_config.h"
#include "base/deps/g3log/scoped_g3log_initializer.h"
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
  // TODO(dimhotepus): Show fancy UI box.
  G3PLOGE_IF(FATAL, !!rc, rc) << "Can't get current directory.  Unable to load "
                                 "the app.  Please, contact authors.";

  app_path /= "libbootmgr.so." WB_PRODUCT_VERSION_INFO_STRING;

  const std::string bootmgr_path{app_path.string()};
  const int bootmgr_load_flags{RTLD_LAZY | RTLD_LOCAL};

  const auto bootmgr_load_result =
      unique_module_ptr::FromLibraryOnPath(bootmgr_path, bootmgr_load_flags);
  if (const auto* bootmgr_module =
          std_ext::GetSuccessResult(bootmgr_load_result)) {
    using BootmgrMainFunction = decltype(&BootmgrMain);

    constexpr char kBootmgrMainFunctionName[]{"BootmgrMain"};

    // Good, try to find and launch bootmgr.
    const auto bootmgr_entry_result =
        bootmgr_module->GetAddressAs<BootmgrMainFunction>(
            kBootmgrMainFunctionName);
    if (const auto* bootmgr_main =
            std_ext::GetSuccessResult(bootmgr_entry_result)) {
      return (*bootmgr_main)({WB_PRODUCT_FILE_DESCRIPTION_STRING, argv, argc});
    }

    // TODO(dimhotepus): Show fancy UI box.
    rc = std::get<std::error_code>(bootmgr_entry_result);
    G3PLOG_E(WARNING, rc)
        << "Can't get '" << kBootmgrMainFunctionName << "' entry point from '"
        << bootmgr_path
        << "'.  Looks like app is broken, please, reinstall the one.";
  } else {
    // TODO(dimhotepus): Show fancy UI box.
    rc = std::get<std::error_code>(bootmgr_load_result);
    G3PLOG_E(WARNING, rc) << "Can't load boot manager '" << bootmgr_path
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

#ifdef WB_OS_LINUX
#if defined(WB_LIBC_GLIBC) && defined(_GLIBCXX_RELEASE)
  G3LOG(INFO) << WB_PRODUCT_FILE_DESCRIPTION_STRING << " build using glibc "
              << __GLIBC__ << "." << __GLIBC_MINOR__ << ", glibc++ "
              << _GLIBCXX_RELEASE << ", ABI stamp " << __GLIBCXX__ << ".";
#endif
#ifdef _LIBCPP_VERSION
  G3LOG(INFO) << WB_PRODUCT_FILE_DESCRIPTION_STRING << " build using libc++ "
              << _LIBCPP_VERSION << "/ ABI " << _LIBCPP_ABI_VERSION;
#endif
#endif

  return BootmgrStartup(argc, argv);
}