// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Toggles window full/narrow screen.

#include "full_screen_window_toggler.h"

#include <system_error>

#include "base/deps/g3log/g3log.h"
#include "base/windows/error_handling/scoped_thread_last_error.h"
#include "base/windows/system_error_ext.h"

namespace wb::base::windows::ui {
FullScreenWindowToggler::FullScreenWindowToggler(
    _In_ HWND window, _In_ LONG default_window_style) noexcept
    : window_{window},
      default_window_style_{default_window_style},
      narrow_window_placement_{sizeof(narrow_window_placement_), 0U, 0U},
      is_fullscreen_now_{false} {
  G3DCHECK(!!window);
}

void FullScreenWindowToggler::Toggle(bool toggle) noexcept {
  if (is_fullscreen_now_ != toggle) {
    const LONG_PTR window_style{::GetWindowLongPtr(window_, GWL_STYLE)};

    if (!is_fullscreen_now_) {
      GoFullScreen(window_style);
    } else {
      GoNarrowScreen(window_style);
    }
  }
}

[[nodiscard]] bool FullScreenWindowToggler::SetWindowStyle(
    _In_ LONG_PTR window_style) const noexcept {
  error_handling::ScopedThreadLastError restore_last_error_on_out;

  // To determine success or failure, clear the last error information by
  // calling SetLastError with 0, then call SetWindowLongPtr.  Function
  // failure will be indicated by a return value of zero and a
  // GetLastError result that is nonzero.
  std_ext::SetThreadErrorCode({});

  const LONG_PTR rc{::SetWindowLongPtr(window_, GWL_STYLE, window_style)};
  const bool ok{rc != 0 || !std_ext::GetThreadErrorCode()};

  G3DCHECK(ok);

  return ok;
}

void FullScreenWindowToggler::GoFullScreen(
    _In_ LONG_PTR window_style) noexcept {
  MONITORINFO mi{sizeof(mi), {}, {}, 0U};

  const bool is_succeeded{
      ::GetWindowPlacement(window_, &narrow_window_placement_) &&
      ::GetMonitorInfo(::MonitorFromWindow(window_, MONITOR_DEFAULTTOPRIMARY),
                       &mi) &&
      SetWindowStyle(window_style &
                     ~static_cast<LONG_PTR>(default_window_style_)) &&
      ::SetWindowPos(window_, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top,
                     mi.rcMonitor.right - mi.rcMonitor.left,
                     mi.rcMonitor.bottom - mi.rcMonitor.top,
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED)};

  G3DCHECK(is_succeeded);

  if (is_succeeded) is_fullscreen_now_ = true;
}

void FullScreenWindowToggler::GoNarrowScreen(
    _In_ LONG_PTR window_style) noexcept {
  const bool is_succeeded{
      SetWindowStyle(window_style |
                     static_cast<LONG_PTR>(default_window_style_)) &&
      ::SetWindowPlacement(window_, &narrow_window_placement_) &&
      ::SetWindowPos(window_, nullptr, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED)};

  G3DCHECK(is_succeeded);

  if (is_succeeded) is_fullscreen_now_ = false;
}
}  // namespace wb::base::windows::ui
