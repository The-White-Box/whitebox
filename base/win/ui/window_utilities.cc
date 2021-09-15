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

namespace wb::base::windows::ui {
/**
 * @brief Moves window to the center of its monitor.
 * @param hwnd Window.
 * @param repaint_after Should repaint window after move?
 * @return true on success, false otherwise.
 */
WB_BASE_API bool MoveWindowToItsDisplayCenter(
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

  const RECT &work_area{mi.rcWork};
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
}  // namespace wb::base::windows::ui
