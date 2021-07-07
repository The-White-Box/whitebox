// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Half-Life 2 launcher app.

#include <tchar.h>  // _T

#include <memory>
#include <system_error>

#include "base/base_macroses.h"
#include "base/deps/g3log/g3log.h"
#include "base/deps/g3log/scoped_g3log_initializer.h"
#include "base/scoped_thread_terminate_handler.h"
#include "base/threads/thread_utils.h"
#include "build/compiler_config.h"
//#include "base/unique_module_ptr.h"
#include "base/windows/com/scoped_com_fatal_exception_handler.h"
#include "base/windows/com/scoped_com_initializer.h"
#include "base/windows/com/scoped_com_strong_unmarshalling_policy.h"
#include "base/windows/error_handling/scoped_thread_error_mode.h"
#include "base/windows/error_handling/scoped_thread_invalid_parameter_handler.h"
#include "base/windows/error_handling/scoped_thread_unexpected_handler.h"
#include "base/windows/memory/memory_utils.h"
#include "base/windows/memory/scoped_new_handler.h"
#include "base/windows/memory/scoped_new_mode.h"
#include "base/windows/scoped_pure_call_handler.h"
#include "base/windows/security/process_mitigations.h"
#include "base/windows/system_error_ext.h"
#include "base/windows/ui/peek_message_dispatcher.h"
#include "base/windows/ui/task_dialog.h"
#include "base/windows/windows_light.h"
#include "base/windows/windows_version.h"
#include "build/static_settings_config.h"
#include "main_window.h"
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
 * @brief Creates main app window definition.
 * @param instance App instance.
 * @return Window definition.
 */
wb::base::windows::ui::WindowDefinition CreateMainWindowDefinition(
    _In_ HINSTANCE instance, _In_z_ PCTSTR window_title, _In_ int width,
    _In_ int height) noexcept {
  G3DCHECK(!!instance);

  const auto icon =
      LoadIcon(instance, MAKEINTRESOURCE(WB_HALF_LIFE_2_IDI_MAIN_ICON));
  const auto icon_small =
      LoadIcon(instance, MAKEINTRESOURCE(WB_HALF_LIFE_2_IDI_SMALL_ICON));
  const auto cursor = LoadCursor(nullptr, IDC_ARROW);
  return wb::base::windows::ui::WindowDefinition{
      instance,
      window_title,
      icon,
      icon_small,
      cursor,
      reinterpret_cast<HBRUSH>(COLOR_APPWORKSPACE),
      WS_OVERLAPPEDWINDOW,
      0,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      width,
      height};
}

/**
 * @brief Run app message loop.
 * @return App exit code.
 */
int DispatchMessages() noexcept {
  int rc{0};
  bool is_done{false};
  const auto handle_quit_message = [&](const MSG& msg) noexcept {
    if (msg.message == WM_QUIT) {
      rc = static_cast<int>(msg.wParam);
      is_done = true;
    }
  };

  using namespace wb::base::windows;
  ui::PeekMessageDispatcher message_dispatcher;

  // Main message app loop.
  while (!is_done) {
    message_dispatcher.Dispatch(ui::HasNoPreDispatchMessage,
                                handle_quit_message);
  }

  return rc;
}

/**
 * @brief Run the app.
 * @param instance App instance.
 * @param show_window_flags Show window flags.
 * @return App exit code.
 */
int RunApp(_In_ HINSTANCE instance, _In_ int show_window_flags) noexcept {
  using namespace wb::base;
  using namespace wb::apps;

  const windows::ui::WindowDefinition window_definition{
      CreateMainWindowDefinition(instance, _T("Half-Life 2 [64 bit]"), 640,
                                 480)};
  constexpr DWORD window_class_style{CS_HREDRAW | CS_VREDRAW};

  auto window_result =
      MainWindow::Create(window_definition, window_class_style);
  if (auto* window_ptr = std::get_if<un<MainWindow>>(&window_result);
      auto* window = window_ptr ? window_ptr->get() : nullptr) {
    // If the window was previously visible, the return value is nonzero.  If
    // the window was previously hidden, the return value is zero.
    ::ShowWindow(window->NativeHandle(), show_window_flags);
    // Send WM_PAINT directly to draw first time.
    ::UpdateWindow(window->NativeHandle());

    return DispatchMessages();
  }

  const auto error_code = std::get<std::error_code>(window_result);
  G3LOG(FATAL) << "Unable to create main '" << window_definition.name
               << "' window: " << error_code.message();

  return error_code.value();
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
                   [[maybe_unused]] _In_ LPSTR command_line,
                   _In_ int show_window_flags) {
#endif
  using namespace wb::base;
  using namespace wb::base::windows;

  // Initialize g3log logging library first as logs are used extensively.
  const deps::g3log::ScopedG3LogInitializer scoped_g3log_initializer{
      ::GetCommandLineA(), wb::build::settings::kPathToMainLogFile};

  // Initialize COM.  Required as ui::ShowDialogBox may call ShellExecute which
  // can delegate execution to shell extensions that are activated using COM.
  const com::ScopedComInitializer scoped_com_initializer{
      com::ScopedComInitializerFlags::kApartmentThreaded |
      com::ScopedComInitializerFlags::kDisableOle1Dde |
      com::ScopedComInitializerFlags::kSpeedOverMemory};
  G3LOG_IF(WARNING, !!scoped_com_initializer.error_code())
      << "COM initialization failed, continue without COM: "
      << scoped_com_initializer.error_code().message();

  // Disallow COM marshalers and unmarshalers not from hardened system-trusted
  // per-process list.
  const com::ScopedComStrongUnmarshallingPolicy
      scoped_com_strong_unmarshalling_policy;
  G3LOG_IF(WARNING, !!scoped_com_strong_unmarshalling_policy.error_code())
      << "Can't enable strong COM unmarshalling policy, some non-trusted "
         "marshallers can be used: "
      << scoped_com_strong_unmarshalling_policy.error_code().message();

  // Calling thread will handle critical errors, does not show general
  // protection fault error box and message box for OpenFile failed to find
  // file.
  const error_handling::ScopedThreadErrorMode scoped_thread_error_mode{
#ifdef NDEBUG
      error_handling::ScopedThreadErrorModeFlags::kFailOnCriticalErrors |
#endif
      error_handling::ScopedThreadErrorModeFlags::kNoGpFaultErrorBox |
      error_handling::ScopedThreadErrorModeFlags::kNoOpenFileErrorBox};
  G3LOG_IF(WARNING, !!scoped_thread_error_mode.error_code())
      << "Can't set thread reaction to serious system errors, continue with "
         "default reaction: "
      << scoped_thread_error_mode.error_code().message();

  // Requires Windows Version 1903+ for UTF-8 process code page.
  //
  // "With a minimum target version of Windows Version 1903, the process code
  // page will always be UTF-8 so legacy code page detection and conversion can
  // be avoided.".
  //
  // See
  // https://docs.microsoft.com/en-us/windows/uwp/design/globalizing/use-utf8-code-page#set-a-process-code-page-to-utf-8
  if (windows::GetVersion() < Version::WIN10_19H1) [[unlikely]] {
    constexpr char kOldWindowsVersionError[]{
        "Windows version is too old.  Version 1903 (May 2019 Update) or "
        "greater is required."};
    G3LOG(WARNING) << kOldWindowsVersionError;

    ui::DialogBoxSettings dialog_settings(
        nullptr, HALF_LIFE_2_ERROR_DIALOG_TITLE_STR,
        "Update Windows to \"Version 1903 (May 2019 Update)\" or greater",
        "Unfortunately your Windows version is too old.  Version 1903 (May "
        "2019 Update) or greater is required.",
        HALF_LIFE_2_ERROR_DIALOG_HIDE_TECH_DETAILS_STR,
        HALF_LIFE_2_ERROR_DIALOG_SEE_TECH_DETAILS_STR,
        std_ext::GetThreadErrorCode(ERROR_OLD_WIN_VERSION).message(),
        wb::build::settings::ui::error_dialog::kFooterLink,
        ui::DialogBoxButton::kOk, false);
    ui::ShowDialogBox(ui::DialogBoxKind::kError, dialog_settings);

    return ERROR_OLD_WIN_VERSION;
  }

  {
    // Terminate the app if system detected heap corruption.
    const auto error_code = memory::EnableTerminationOnHeapCorruption();
    G3LOG_IF(FATAL, !!error_code)
        << "Can't enable 'Terminate on Heap corruption' os feature, continue "
           "without it: "
        << error_code.message();
  }

  {
    // Enable heap resources optimization.
    const auto error_code = memory::EnableHeapResourcesOptimization();
    G3LOG_IF(WARNING, !!error_code)
        << "Can't enable 'heap resources optimization' os feature, some caches "
           "will not be optimized: "
        << error_code.message();
  }

  // Enable process attacks mitigation policies in scope.
  const security::ScopedProcessMitigationPolicies
      scoped_process_mitigation_policies;
  G3LOG_IF(FATAL, !!scoped_process_mitigation_policies.error_code())
      << "Can't enable process attacks mitigation policies, attacker can use "
         "system features to break in app: "
      << scoped_process_mitigation_policies.error_code().message();

  {
    // Search for DLLs in the secure order to prevent DLL plant attacks.
    const auto error_code = security::EnableDefaultSecureDllSearch();
    G3LOG_IF(FATAL, !!error_code)
        << "Can't enable secure DLL search order, attacker can plant DLLs with "
           "malicious code: "
        << error_code.message();
  }

  // Disable default COM exception swallowing, report all COM exceptions to us.
  const com::ScopedComFatalExceptionHandler scoped_com_fatal_exception_handler;
  G3LOG_IF(WARNING, !!scoped_com_fatal_exception_handler.error_code())
      << "Can't disable COM exceptions swallowing, some exceptions may not be "
         "passed to the app: "
      << scoped_com_fatal_exception_handler.error_code().message();

  // Handle call of CRT function with bad arguments on the thread.
  const error_handling::ScopedThreadInvalidParameterHandler
      scoped_thread_invalid_parameter_handler{
          error_handling::DefaultThreadInvalidParameterHandler};
  // Handle pure virtual function call.
  const ScopedPureCallHandler scoped_pure_call_handler{DefaultPureCallHandler};
  // Handle new allocation failure.
  const memory::ScopedNewHandler scoped_new_handler{
      memory::DefaultNewFailureHandler};
  // Call new when malloc failed.
  const memory::ScopedNewMode scoped_new_mode{
      memory::ScopedNewModeFlag::CallNew};
  // Handle terminate function call on the thread.
  const ScopedThreadTerminateHandler scoped_thread_terminate_handler{
      DefaultThreadTerminateHandler};
  // Handle unexpected function call on the thread.
  const error_handling::ScopedThreadUnexpectedHandler
      scoped_thread_unexpected_handler{
          error_handling::DefaultThreadUnexpectedHandler};

  // Mark main thread with name to simplify debugging.
  // Evaluation order doesn't matter in our case as string_view and HANDLE are
  // evaluation independent.
  // C4868: compiler may not enforce left-to-right evaluation order in braced
  // initializer list
  const threads::ScopedThreadName scoped_thread_name(::GetCurrentThread(),
                                                     "WhiteBoxMain");
  G3LOG_IF(WARNING, !!scoped_thread_name.error_code())
      << "Can't rename main thread, continue with default name: "
      << scoped_thread_name.error_code().message();

  return RunApp(instance, show_window_flags);
}