// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Scoped cursor toggler.

#ifndef WB_BASE_INCLUDE_WINDOWS_UI_SCOPED_TOGGLE_CURSOR_H_
#define WB_BASE_INCLUDE_WINDOWS_UI_SCOPED_TOGGLE_CURSOR_H_
#ifdef _WIN32
#pragma once
#endif

#include "base/include/base_macroses.h"

__declspec(dllimport) int __stdcall ShowCursor(_In_ int bShow);

namespace wb::base::windows::ui {
/**
 * @brief Hides cursor in scope and reverts back when out of scope.
 */
class ScopedToggleCursor {
 public:
  /**
   * @brief Creates scoped cursor and toggles it.
   * @param toggle Toggle for cursor.
   * @return nothing.
   */
  explicit ScopedToggleCursor(_In_ bool toggle) noexcept
      : display_cursor_counter_{::ShowCursor(toggle ? 1 : 0)},
        is_show_cursor_{toggle} {}

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedToggleCursor);

  ~ScopedToggleCursor() noexcept {
    ::ShowCursor(is_show_cursor_ ? 0 : 1);
  }

  [[nodiscard]] bool IsVisible() const noexcept {
    // If a mouse is installed, the initial display count is 0.  If no mouse is
    // installed, the display count is –1.
    return display_cursor_counter_ >= 0;
  }

 private:
  const int display_cursor_counter_;
  const bool is_show_cursor_;
};
}  // namespace wb::base::windows::ui

#endif  // !WB_BASE_INCLUDE_WINDOWS_UI_SCOPED_TOGGLE_CURSOR_H_
