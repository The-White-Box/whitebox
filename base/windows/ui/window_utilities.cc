// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Common window utilities.

#include "window_utilities.h"

#include <algorithm>

#include "base/base_macroses.h"
#include "base/deps/g3log/g3log.h"
#include "base/windows/windows_light.h"
#include "build/compiler_config.h"

namespace wb::base::windows::ui {
/**
 * @brief Moves window to the center of the primary monitor.
 * @param hwnd Window.
 * @param repaint_after Should repaint window after move?
 * @return true on success, false otherwise.
 */
WB_BASE_API bool MoveWindowToPrimaryDisplayCenter(
    _In_ HWND window, _In_ bool repaint_after) noexcept {
  G3DCHECK(!!window);

  RECT window_rect;
  [[maybe_unused]] bool is_ok{!!::GetWindowRect(window, &window_rect)};
  G3DCHECK(is_ok);

  const int screen_width{::GetSystemMetrics(SM_CXSCREEN)},
      screen_height{::GetSystemMetrics(SM_CYSCREEN)};
  const int window_width{std::min(wb::base::implicit_cast<long>(screen_width),
                                  window_rect.right - window_rect.left)},
      window_height{std::min(wb::base::implicit_cast<long>(screen_height),
                             window_rect.bottom - window_rect.top)};
  const int x_pos{(screen_width - window_width) / 2},
      y_pos{(screen_height - window_height) / 2};

  is_ok = !!::MoveWindow(window, x_pos, y_pos, window_width, window_height,
                         repaint_after ? TRUE : FALSE);
  G3DCHECK(is_ok);

  return is_ok;
}
}  // namespace wb::base::windows::ui
