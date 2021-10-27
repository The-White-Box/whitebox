// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Common window utilities.

#include "window_utilities.h"

#include <algorithm>

#include "base/base_macroses.h"
#include "base/deps/g3log/g3log.h"
#include "base/win/windows_light.h"
#include "build/compiler_config.h"

namespace wb::ui::win {

/**
 * @brief Moves window to the center of its monitor.
 * @param hwnd Window.
 * @param repaint_after Should repaint window after move?
 * @return true on success, false otherwise.
 */
WB_WHITEBOX_UI_API bool MoveWindowToItsDisplayCenter(
    _In_ HWND window, _In_ bool repaint_after) noexcept {
  G3DCHECK(!!window);

  RECT window_rect;
  [[maybe_unused]] bool is_ok{!!::GetWindowRect(window, &window_rect)};
  G3DCHECK(is_ok);

  const HMONITOR monitor{::MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY)};
  G3DCHECK(!!monitor);
  if (!monitor) return false;

  MONITORINFO mi{.cbSize = static_cast<DWORD>(sizeof(mi))};
  is_ok = !!::GetMonitorInfoA(monitor, &mi);
  G3DCHECK(is_ok);

  const RECT& work_area{mi.rcWork};
  const long screen_width{work_area.right - work_area.left},
      screen_height{work_area.bottom - work_area.top};

  const int window_width{
      std::min(screen_width, window_rect.right - window_rect.left)},
      window_height{
          std::min(screen_height, window_rect.bottom - window_rect.top)};
  const int x_pos{(screen_width - window_width) / 2},
      y_pos{(screen_height - window_height) / 2};

  is_ok = !!::MoveWindow(window, x_pos, y_pos, window_width, window_height,
                         repaint_after ? TRUE : FALSE);
  G3DCHECK(is_ok);

  return is_ok;
}

/**
 * @brief Flashes window caption and title bar.
 * @param window_class_name Window class name.
 * @param timeout_between_flashes How many milliseconds to wait between window
 * flashes.
 * @return true if window is flashing, false otherwise.
 */
WB_WHITEBOX_UI_API bool FlashWindowByClass(
    _In_ const std::string& window_class_name,
    _In_ std::chrono::milliseconds timeout_between_flashes) noexcept {
  G3DCHECK(!window_class_name.empty());
  G3DCHECK(timeout_between_flashes.count() >= 0 &&
           timeout_between_flashes.count() <=
               std::numeric_limits<decltype(FLASHWINFO::dwTimeout)>::max());

  const HWND hwnd_to_flash{::FindWindowA(window_class_name.c_str(), nullptr)};
  if (hwnd_to_flash) {
    FLASHWINFO flash_info{
        .cbSize = sizeof(flash_info),
        .hwnd = hwnd_to_flash,
        .dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG,
        .uCount = 7,
        .dwTimeout = static_cast<decltype(FLASHWINFO::dwTimeout)>(
            timeout_between_flashes.count())};
    // The return value specifies the window's state before the call to the
    // FlashWindowEx function.  If the window caption was drawn as active before
    // the call, the return value is nonzero.  Otherwise, the return value is
    // zero.
    //
    // See
    // https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-flashwindowex#return-value
    ::FlashWindowEx(&flash_info);
    return true;
  }

  G3DLOG(WARNING) << "No window with class name " << window_class_name
                  << " was found.  Nothing will be flashed.";
  return false;
}

}  // namespace wb::ui::win
