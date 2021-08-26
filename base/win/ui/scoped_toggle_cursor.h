// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Scoped window cursor visibility toggler.

#ifndef WB_BASE_WIN_UI_SCOPED_TOGGLE_CURSOR_H_
#define WB_BASE_WIN_UI_SCOPED_TOGGLE_CURSOR_H_

#include "base/base_macroses.h"
#include "build/compiler_config.h"

WB_ATTRIBUTE_DLL_IMPORT int __stdcall ShowCursor(_In_ int bShow);

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

  /**
   * @brief Decrement cursor view counter.
   */
  ~ScopedToggleCursor() noexcept { ::ShowCursor(is_show_cursor_ ? 0 : 1); }

  /**
   * @brief Is cursor visible?
   * @return true if visible, false otherwise.
   */
  [[nodiscard]] bool IsVisible() const noexcept {
    // If a mouse is installed, the initial display count is 0.  If no mouse is
    // installed, the display count is –1.
    return display_cursor_counter_ >= 0;
  }

 private:
  /**
   * @brief Counter of cursor display / hide calls.  If >= 0 than cursor
   * visible.
   */
  const int display_cursor_counter_;
  /**
   * @brief Should try to show cursor or not.
   */
  const bool is_show_cursor_;
};
}  // namespace wb::base::windows::ui

#endif  // !WB_BASE_WIN_UI_SCOPED_TOGGLE_CURSOR_H_
