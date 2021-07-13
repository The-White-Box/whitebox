// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Half-Life 2 launcher app.

#include <tchar.h>

#include <filesystem>
#include <system_error>

#include "base/deps/g3log/scoped_g3log_initializer.h"
#include "base/unique_module_ptr.h"
#include "base/windows/com/scoped_com_fatal_exception_handler.h"
#include "base/windows/com/scoped_com_initializer.h"
#include "base/windows/com/scoped_com_strong_unmarshalling_policy.h"
#include "base/windows/error_handling/scoped_thread_error_mode.h"
#include "base/windows/windows_light.h"
#include "bootmgr/bootmgr_main.h"
#include "build/compiler_config.h"  // WB_ATTRIBUTE_DLL_EXPORT
#include "build/static_settings_config.h"
#include "windows_resource.h"

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
int BootmgrStartup(_In_ HINSTANCE instance, _In_ LPTSTR command_line,
                   _In_ int show_window_flags) noexcept {
  std::error_code rc;
  std::filesystem::path bootmgr_path{std::filesystem::current_path(rc)};
  G3PLOGE_IF(FATAL, !!rc, rc) << "Can't get current directory.  Unable to load "
                                 "the app.  Please, contact authors.";

  bootmgr_path /= "bootmgr.dll";

  constexpr char kBootmgrMainFunctionName[]{"BootmgrMain"};

  // TODO(dimhotepus): Correct flags + LOAD_LIBRARY_REQUIRE_SIGNED_TARGET.
  const auto bootmgr_load_result =
      wb::base::unique_module_ptr::from_load_library(
          bootmgr_path.string(), LOAD_WITH_ALTERED_SEARCH_PATH);
  if (const auto* bootmgr_module =
          std::get_if<wb::base::unique_module_ptr>(&bootmgr_load_result)) {
    using BootmgrMainFunction = decltype(&BootmgrMain);

    // Good, try to find and launch bootmgr.
    const auto bootmgr_entry_result =
        bootmgr_module->get_address_as<BootmgrMainFunction>(
            kBootmgrMainFunctionName);
    if (const auto* bootmgr_main =
            std::get_if<BootmgrMainFunction>(&bootmgr_entry_result)) {
      return (*bootmgr_main)({instance, command_line,
                              WB_APP_VER_FILE_DESCRIPTION_STR,
                              show_window_flags, WB_HALF_LIFE_2_IDI_MAIN_ICON,
                              WB_HALF_LIFE_2_IDI_SMALL_ICON});
    }

    // TODO(dimhotepus): Show fancy Ui message box.
    rc = std::get<std::error_code>(bootmgr_entry_result);
    G3PLOG_E(WARNING, rc)
        << "Can't get '" << kBootmgrMainFunctionName << "' entry point from '"
        << bootmgr_path.string()
        << "'.  Looks like app is broken, please, reinstall the one.";
  } else {
    // TODO(dimhotepus): Show fancy Ui message box.
    rc = std::get<std::error_code>(bootmgr_load_result);
    G3PLOG_E(WARNING, rc) << "Can't load boot manager '"
                          << bootmgr_path.string()
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
#ifdef UNICODE
int WINAPI wWinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE,
                    [[maybe_unused]] _In_ LPWSTR command_line,
                    _In_ int show_window_flags) {
#else
int WINAPI WinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE,
                   _In_ LPSTR command_line, _In_ int show_window_flags) {
#endif
  using namespace wb::base;
  using namespace wb::base::windows;

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
  G3PLOGE_IF(WARNING, !!std::get_if<std::error_code>(&scoped_thread_error_mode),
             std::get<std::error_code>(scoped_thread_error_mode))
      << "Can't set thread reaction to serious system errors, continue with "
         "default reaction.";

  // Initialize COM.  Required as ui::ShowDialogBox may call ShellExecute which
  // can delegate execution to shell extensions that are activated using COM.
  const auto scoped_com_initializer = com::ScopedComInitializer::New(
      com::ScopedComInitializerFlags::kApartmentThreaded |
      com::ScopedComInitializerFlags::kDisableOle1Dde |
      com::ScopedComInitializerFlags::kSpeedOverMemory);
  G3PLOGE_IF(WARNING, !!std::get_if<std::error_code>(&scoped_com_initializer),
             std::get<std::error_code>(scoped_com_initializer))
      << "Component Object Model initialization failed, continue without COM.";

  // Disable default COM exception swallowing, report all COM exceptions to us.
  const com::ScopedComFatalExceptionHandler scoped_com_fatal_exception_handler;
  G3PLOGE_IF(WARNING, !!scoped_com_fatal_exception_handler.error_code(),
             scoped_com_fatal_exception_handler.error_code())
      << "Can't disable COM exceptions swallowing, some exceptions may not be "
         "passed to the app.";

  // Disallow COM marshalers and unmarshalers not from hardened system-trusted
  // per-process list.
  const com::ScopedComStrongUnmarshallingPolicy
      scoped_com_strong_unmarshalling_policy;
  G3PLOGE_IF(WARNING, !!scoped_com_strong_unmarshalling_policy.error_code(),
             scoped_com_strong_unmarshalling_policy.error_code())
      << "Can't enable strong COM unmarshalling policy, some non-trusted "
         "marshallers can be used.";

  return BootmgrStartup(instance, command_line, show_window_flags);
}