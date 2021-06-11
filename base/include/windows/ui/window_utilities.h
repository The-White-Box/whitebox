// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Common window utilities.

#ifndef WB_BASE_INCLUDE_WINDOW_UTILITIES_H_
#define WB_BASE_INCLUDE_WINDOW_UTILITIES_H_
#ifdef _WIN32
#pragma once
#endif

#include <algorithm>

#include "base/include/base_macroses.h"
#include "base/include/deps/g3log/g3log.h"
#include "base/include/windows/windows_light.h"

namespace wb::base::windows::ui {
/**
 * @brief Moves window to the center of the primary monitor.
 * @param hwnd Window.
 * @param repaint_after Should repaint window after move?
 * @return true on success, false otherwise.
 */
inline bool MoveWindowToPrimaryDisplayCenter(_In_ HWND window,
                                             _In_ bool repaint_after) noexcept {
  DCHECK(!!window);

  RECT window_rect;
  [[maybe_unused]] bool is_ok{!!::GetWindowRect(window, &window_rect)};
  DCHECK(is_ok);

  const int screen_width{::GetSystemMetrics(SM_CXSCREEN)},
      screen_height{::GetSystemMetrics(SM_CYSCREEN)};
  const int window_width{std::min(
      wb::base::implicit_cast<long>(screen_width), window_rect.right)},
      window_height{std::min(wb::base::implicit_cast<long>(screen_height),
                             window_rect.bottom)};
  const int x_pos{(screen_width - window_width) / 2},
      y_pos{(screen_height - window_height) / 2};

  is_ok = !!::MoveWindow(window, x_pos, y_pos, window_width, window_height,
                         repaint_after ? TRUE : FALSE);
  DCHECK(is_ok);

  return is_ok;
}
}  // namespace wb::base::windows::ui

#endif  // !WB_BASE_INCLUDE_WINDOW_UTILITIES_H_
