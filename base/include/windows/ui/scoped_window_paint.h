// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Scoped window paint.

#ifndef WHITEBOX_BASE_INCLUDE_WINDOWS_UI_SCOPED_WINDOW_PAINT_H_
#define WHITEBOX_BASE_INCLUDE_WINDOWS_UI_SCOPED_WINDOW_PAINT_H_
#ifdef _WIN32
#pragma once
#endif

#include "base/include/base_macroses.h"
#include "base/include/deps/g3log/g3log.h"
#include "base/include/windows/windows_light.h"

namespace whitebox::base::windows::ui {
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
    DCHECK(!!window_);
    DCHECK(!!device_context_);
  }

  WHITEBOX_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedWindowPaint);

  ~ScopedWindowPaint() noexcept {
    if (device_context_) {
      // The return value is always nonzero.
      //
      // See
      // https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-endpaint
      (void)::EndPaint(window_, &paint_struct_);
    }
  }

 private:
  const HWND window_;
  const HDC device_context_;
  PAINTSTRUCT paint_struct_;
};
}  // namespace whitebox::base::windows::ui

#endif  // !WHITEBOX_BASE_INCLUDE_WINDOWS_UI_SCOPED_WINDOW_PAINT_H_
