// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// The entry point for windows Half-Life 2 process.

#include <filesystem>
#include <system_error>

#include "base/base_switches.h"
#include "base/deps/g3log/scoped_g3log_initializer.h"
#include "base/unique_module_ptr.h"
#include "base/windows/com/scoped_com_fatal_exception_handler.h"
#include "base/windows/com/scoped_com_initializer.h"
#include "base/windows/com/scoped_com_strong_unmarshalling_policy.h"
#include "base/windows/dll_load_utils.h"
#include "base/windows/error_handling/scoped_thread_error_mode.h"
#include "base/windows/windows_light.h"
#include "bootmgr/bootmgr_main.h"
#include "build/compiler_config.h"  // WB_ATTRIBUTE_DLL_EXPORT
#include "build/static_settings_config.h"
#include "resource_win.h"

extern "C" {
// Starting with the Release 302 drivers, application developers can direct the
// Optimus driver at runtime to use the High Performance Graphics to render any
// application–even those applications for which there is no existing
// application profile.
//
// See
// https://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
WB_ATTRIBUTE_DLL_EXPORT DWORD NvOptimusEnablement = 0x00000001;

// This will select the high performance GPU as long as no profile exists that
// assigns the application to another GPU.  Please make sure to use a 13.35 or
// newer driver.  Older drivers do not support this.
//
// See
// https://community.amd.com/t5/firepro-development/can-an-opengl-app-default-to-the-discrete-gpu-on-an-enduro/td-p/279440
WB_ATTRIBUTE_DLL_EXPORT int AmdPowerXpressRequestHighPerformance = 0x00000001;
}

namespace {
/**
 * @brief Load and run bootmgr.
 * @param instance App instance.
 * @param command_line Command line.
 * @param show_window_flags Show window flags.
 * @return App exit code.
 */
int BootmgrStartup(_In_ HINSTANCE instance, _In_ LPCSTR command_line,
                   _In_ int show_window_flags) noexcept {
  using namespace wb::base;

  // Search for DLLs in the secure order to prevent DLL plant attacks.
  std::error_code rc{windows::GetErrorCode(::SetDefaultDllDirectories(
      LOAD_LIBRARY_SEARCH_SYSTEM32 | LOAD_LIBRARY_SEARCH_USER_DIRS))};
  G3PLOGE_IF(WARNING, !!rc, rc)
      << "Can't enable secure DLL search order, attacker can plant DLLs with "
         "malicious code.";

  const auto app_path = windows::GetApplicationDirectory(instance);
  G3PLOGE_IF(FATAL, !!wb::base::std_ext::GetErrorCode(app_path),
             std::get<std::error_code>(app_path))
      << "Can't get current directory.  Unable to load "
         "the app.  Please, contact authors.";

  const std::string bootmgr_path{std::get<std::string>(app_path) +
                                 "bootmgr.dll"};
  const unsigned bootmgr_load_flags{
      LOAD_WITH_ALTERED_SEARCH_PATH |
      (windows::MustBeSignedDllLoadTarget(command_line)
           ? LOAD_LIBRARY_REQUIRE_SIGNED_TARGET
           : 0U)};

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
      return (*bootmgr_main)({instance, command_line,
                              WB_PRODUCT_FILE_DESCRIPTION_STRING,
                              show_window_flags, WB_HALF_LIFE_2_IDI_MAIN_ICON,
                              WB_HALF_LIFE_2_IDI_SMALL_ICON});
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

/**
 * @brief Windows app entry point.
 * @param instance App instance.
 * @param command_line Command line.
 * @param show_window_flags Show window flags.
 * @return App exit code.
 */
int WINAPI WinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE,
                   _In_ LPSTR command_line, _In_ int show_window_flags) {
  using namespace wb::base;
  using namespace wb::base::windows;

#ifdef _DEBUG
  // Simplifies debugging experience, no need to sign targets.
  std::string debug_command_line{command_line};
  debug_command_line.append(" ");
  debug_command_line.append(
      wb::base::switches::kUnsafeAllowUnsignedModuleTargetFlag);

  command_line = debug_command_line.data();
#endif

  // Initialize g3log logging library first as logs are used extensively.
  const deps::g3log::ScopedG3LogInitializer scoped_g3log_initializer{
      ::GetCommandLineA(), wb::build::settings::kPathToMainLogFile};

  // Calling thread will handle critical errors, does not show general
  // protection fault error box and message box for OpenFile failed to find
  // file.
  const auto scoped_thread_error_mode =
      error_handling::ScopedThreadErrorMode::New(
#ifdef NDEBUG
          error_handling::ScopedThreadErrorModeFlags::kFailOnCriticalErrors |
#endif
          error_handling::ScopedThreadErrorModeFlags::kNoGpFaultErrorBox |
          error_handling::ScopedThreadErrorModeFlags::kNoOpenFileErrorBox);
  G3PLOGE_IF(WARNING, !!std_ext::GetErrorCode(scoped_thread_error_mode),
             *std_ext::GetErrorCode(scoped_thread_error_mode))
      << "Can't set thread reaction to serious system errors, continue with "
         "default reaction.";

  // Initialize COM.  Required as ui::ShowDialogBox may call ShellExecute which
  // can delegate execution to shell extensions that are activated using COM.
  const auto scoped_com_initializer = com::ScopedComInitializer::New(
      com::ScopedComInitializerFlags::kApartmentThreaded |
      com::ScopedComInitializerFlags::kDisableOle1Dde |
      com::ScopedComInitializerFlags::kSpeedOverMemory);
  G3PLOGE_IF(WARNING, !!std_ext::GetErrorCode(scoped_com_initializer),
             *std_ext::GetErrorCode(scoped_com_initializer))
      << "Component Object Model initialization failed, continue without COM.";

  // Disable default COM exception swallowing, report all COM exceptions to us.
  const auto scoped_com_fatal_exception_handler =
      com::ScopedComFatalExceptionHandler::New();
  G3PLOGE_IF(WARNING,
             !!std_ext::GetErrorCode(scoped_com_fatal_exception_handler),
             *std_ext::GetErrorCode(scoped_com_fatal_exception_handler))
      << "Can't disable COM exceptions swallowing, some exceptions may not be "
         "passed to the app.";

  // Disallow COM marshalers and unmarshalers not from hardened system-trusted
  // per-process list.
  const auto scoped_com_strong_unmarshalling_policy =
      com::ScopedComStrongUnmarshallingPolicy::New();
  G3PLOGE_IF(WARNING,
             !!std_ext::GetErrorCode(scoped_com_strong_unmarshalling_policy),
             *std_ext::GetErrorCode(scoped_com_strong_unmarshalling_policy))
      << "Can't enable strong COM unmarshalling policy, some non-trusted "
         "marshallers can be used.";

  return BootmgrStartup(instance, command_line, show_window_flags);
}