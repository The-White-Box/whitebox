// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Scoped window painting on GDI device context.

#ifndef WB_BASE_WINDOWS_UI_SCOPED_WINDOW_PAINT_H_
#define WB_BASE_WINDOWS_UI_SCOPED_WINDOW_PAINT_H_

#include "base/base_macroses.h"
#include "base/deps/g3log/g3log.h"
#include "base/windows/windows_light.h"

namespace wb::base::windows::ui {
/**
 * @brief Paints to window in scope.
 */
class ScopedWindowPaint {
 public:
  /**
   * @brief Creates scoped paint window context.
   * @param window Window to paint on.
   * @return nothing.
   */
  explicit ScopedWindowPaint(_In_ HWND window) noexcept
      : window_{window}, device_context_{::BeginPaint(window, &paint_struct_)} {
    G3DCHECK(!!window_);
    G3DCHECK(!!device_context_);
  }

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedWindowPaint);

  /**
   * @brief Closes painting device.
   */
  ~ScopedWindowPaint() noexcept {
    if (device_context_) {
      // The return value is always nonzero.
      //
      // See
      // https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-endpaint
      G3CHECK(!!::EndPaint(window_, &paint_struct_));
    }
  }

  /**
   * @brief Draws text.
   * @param text Text to draw.
   * @param size Text size.
   * @param rc Rectangle to draw text into.
   * @param format Text format flags.
   * @return Height of the drawn text in logical units.
   */
  int TextDraw(LPCSTR text, int size, RECT* rc, UINT format) noexcept {
    G3DCHECK(!!device_context_);
    G3DCHECK(!!rc);

    const int height_in_logical_units{
        ::DrawTextA(device_context_, text, size, rc, format)};
    G3DCHECK(!!height_in_logical_units);

    return height_in_logical_units;
  }

  /**
   * @brief Paint information.
   * @return PAINTSTRUCT.
   */
  [[nodiscard]] const PAINTSTRUCT& PaintInfo() const noexcept {
    return paint_struct_;
  }

 private:
  /**
   * @brief Window.
   */
  const HWND window_;
  /**
   * @brief Window device context.
   */
  const HDC device_context_;
  /**
   * @brief Paint info.
   */
  PAINTSTRUCT paint_struct_;
};
}  // namespace wb::base::windows::ui

#endif  // !WB_BASE_WINDOWS_UI_SCOPED_WINDOW_PAINT_H_
