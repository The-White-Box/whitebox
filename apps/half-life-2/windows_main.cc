// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Half-Life 2 launcher app.

#include <tchar.h>  // _T

#include <memory>
#include <system_error>

#include "base/include/base_macroses.h"
#include "base/include/deps/g3log/g3log.h"
#include "base/include/deps/g3log/scoped_g3log_initializer.h"
#include "base/include/deps/mimalloc/mimalloc.h"
#include "base/include/scoped_thread_terminate_handler.h"
#include "base/include/windows/scoped_minimum_timer_resolution.h"
#include "base/include/windows/scoped_new_handler.h"
#include "base/include/windows/scoped_new_mode.h"
#include "base/include/windows/scoped_pure_call_handler.h"
#include "base/include/windows/scoped_thread_invalid_parameter_handler.h"
#include "base/include/windows/scoped_thread_unexpected_handler.h"
#include "base/include/windows/ui/peek_message_dispatcher.h"
#include "base/include/windows/windows_light.h"
#include "build/include/static_settings_config.h"
#include "main_window.h"
#include "windows_resource.h"

#ifdef WB_OS_WIN
extern "C" {
// Starting with the Release 302 drivers, application developers can direct the
// Optimus driver at runtime to use the High Performance Graphics to render any
// application–even those applications for which there is no existing
// application profile.
//
// See
// https://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;

// This will select the high performance GPU as long as no profile exists that
// assigns the application to another GPU.  Please make sure to use a 13.35 or
// newer driver.  Older drivers do not support this.
//
// See
// https://community.amd.com/t5/firepro-development/can-an-opengl-app-default-to-the-discrete-gpu-on-an-enduro/td-p/279440
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 0x00000001;
}
#endif

namespace {
/**
 * @brief Creates main app window definition.
 * @param instance App instance.
 * @return Window definition.
 */
wb::base::windows::ui::WindowDefinition CreateMainWindowDefinition(
    _In_ HINSTANCE instance, _In_z_ PCTSTR window_title) noexcept {
  DCHECK(!!instance);

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
      640,
      480};
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
      CreateMainWindowDefinition(instance, _T("Half-Life 2 [64 bit]"))};
  constexpr DWORD window_class_style{CS_HREDRAW | CS_VREDRAW};

  auto window_result =
      MainWindow::Create(window_definition, window_class_style);
  if (auto* window_ptr = std::get_if<U<MainWindow>>(&window_result);
      auto* window = window_ptr ? window_ptr->get() : nullptr) {
    // If the window was previously visible, the return value is nonzero.  If
    // the window was previously hidden, the return value is zero.
    (void)::ShowWindow(window->NativeHandle(), show_window_flags);
    // Send WM_PAINT directly to draw first time.
    (void)::UpdateWindow(window->NativeHandle());

    return DispatchMessages();
  }

  return std::get<std::error_code>(window_result).value();
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
  // Ignore earlier allocations.
  mi_stats_reset();

  // TODO(dimhotepus): Check OS is Windows Version 1903+ (May 2019 Update) for
  // utf-8.  See
  // https://docs.microsoft.com/en-us/windows/uwp/design/globalizing/use-utf8-code-page#set-a-process-code-page-to-utf-8

  using namespace wb::base;

  // Initialize g3log logging library first as logs used extensively.
  deps::g3log::ScopedG3LogInitializer scoped_g3log_initializer{
      ::GetCommandLineA(), wb::build::settings::kPathToMainLogFile};

  DLOG(INFO) << "Using mi-malloc v." << mi_version();

  // Handle call of CRT function with bad arguments on the thread.
  windows::ScopedThreadInvalidParameterHandler
      scoped_thread_invalid_parameter_handler{
          windows::DefaultThreadInvalidParameterHandler};
  // Handle pure virtual function call.
  windows::ScopedPureCallHandler scoped_pure_call_handler{
      windows::DefaultPureCallHandler};
  // Handle new allocation failure.
  windows::ScopedNewHandler scoped_new_handler{
      windows::DefaultNewFailureHandler};
  // Call new when malloc failed.
  windows::ScopedNewMode scoped_new_mode{windows::ScopedNewModeFlag::CallNew};
  // Handle terminate function call on the thread.
  ScopedThreadTerminateHandler scoped_thread_terminate_handler{
      DefaultThreadTerminateHandler};
  // Handle unexpected function call on the thread.
  windows::ScopedThreadUnexpectedHandler scoped_thread_unexpected_handler{
      windows::DefaultThreadUnexpectedHandler};

  // Set minimum timers resolution to good enough, but not too power hungry.
  windows::ScopedMinimumTimerResolution scoped_minimum_timer_resolution{
      wb::build::settings::kMinimumTimerResolutionMs};
  LOG_IF(WARNING, !scoped_minimum_timer_resolution.IsSucceeded())
      << "Failed to set minimum periodic timers resolution to "
      << wb::build::settings::kMinimumTimerResolutionMs
      << "ms, will run with default system one.";

  return RunApp(instance, show_window_flags);
}