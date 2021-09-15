// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Scoped window painting on GDI device context.

#ifndef WB_BASE_WIN_UI_SCOPED_WINDOW_PAINT_H_
#define WB_BASE_WIN_UI_SCOPED_WINDOW_PAINT_H_

#include "base/base_api.h"
#include "base/base_macroses.h"
#include "base/std_ext/system_error_ext.h"
#include "build/compiler_config.h"

using HWND = struct HWND__*;
using PAINTSTRUCT = struct tagPAINTSTRUCT;
using RECT = struct tagRECT;

namespace wb::base::windows::ui {
/**
 * @brief Paints to window in the scope.
 */
class WB_BASE_API ScopedWindowPaint {
 public:
  /**
   * @brief Creates scoped window painter.
   * @param window Window.
   * @return ScopedWindowPaint.
   */
  [[nodiscard]] static std_ext::os_res<ScopedWindowPaint> New(
      _In_ HWND window) noexcept;

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(ScopedWindowPaint);

  ScopedWindowPaint(ScopedWindowPaint&&) noexcept;
  ScopedWindowPaint& operator=(ScopedWindowPaint&&) noexcept = delete;

  /**
   * @brief Closes painting device.
   */
  ~ScopedWindowPaint() noexcept;

  /**
   * @brief Draws text.
   * @param text Text to draw.
   * @param size Text size.
   * @param rc Rectangle to draw text into.
   * @param format Text format flags.
   * @return Height of the drawn text in logical units.
   */
  int TextDraw(const char* text, int size, RECT* rc,
               unsigned format) const noexcept;

  /**
   * @brief Paints the specified rectangle using the brush that is currently
   * selected into the specified device context.  The brush color and the
   * surface color or colors are combined by using the specified raster
   * operation.
   * @param rc Rectangle in logical units to fill.
   * @param raster_operation Raster operation code.
   * @return true on success, false otherwise.
   */
  bool BlitPattern(const RECT& rc,
                   unsigned long raster_operation) const noexcept;

  /**
   * @brief Paint information.
   * @return PAINTSTRUCT.
   */
  [[nodiscard]] const PAINTSTRUCT& PaintInfo() const noexcept;

 private:
  class ScopedWindowPaintImpl;

  WB_COMPILER_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()
    // Private member is not accessible to the DLL's client, including inline
    // functions.
    WB_COMPILER_MSVC_DISABLE_WARNING(4251)
    /**
     * @brief Actual implementation.
     */
    wb::base::un<ScopedWindowPaintImpl> impl_;
  WB_COMPILER_MSVC_END_WARNING_OVERRIDE_SCOPE()

  /**
   * @brief Creates scoped paint window context.
   * @param window Window to paint on.
   * @return nothing.
   */
  explicit ScopedWindowPaint(_In_ HWND window) noexcept;
};
}  // namespace wb::base::windows::ui

#endif  // !WB_BASE_WIN_UI_SCOPED_WINDOW_PAINT_H_
