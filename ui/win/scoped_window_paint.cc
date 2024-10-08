// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Scoped window painting on GDI device context.

#include "scoped_window_paint.h"

#include "base/deps/g3log/g3log.h"
#include "base/macroses.h"
#include "base/std2/system_error_ext.h"
#include "base/win/windows_light.h"

namespace wb::ui::win {

/**
 * @brief Implementation of scoped window painter.
 */
class ScopedWindowPaint::ScopedWindowPaintImpl final {
 public:
  /**
   * @brief Creates scoped paint window context.
   * @param window Window to paint on.
   * @return nothing.
   */
  explicit ScopedWindowPaintImpl(_In_ HWND window) noexcept
      : window_{window}, device_context_{::BeginPaint(window, &paint_struct_)} {
    G3DCHECK(!!window_);
    G3DCHECK(!!device_context_);
  }

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedWindowPaintImpl);

  /**
   * @brief Closes painting device.
   */
  ~ScopedWindowPaintImpl() noexcept {
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
  int TextDraw(LPCSTR text, int size, RECT* rc, UINT format) const noexcept {
    G3DCHECK(!!device_context_);
    G3DCHECK(!!rc);

    const int height_in_logical_units{
        ::DrawTextA(device_context_, text, size, rc, format)};
    G3DCHECK(!!height_in_logical_units);

    return height_in_logical_units;
  }

  /**
   * @brief Paints the specified rectangle using the brush that is currently
   * selected into the specified device context.  The brush color and the
   * surface color or colors are combined by using the specified raster
   * operation.
   * @param rc Rectangle in logical units to fill.
   * @param raster_operation Raster operation code.
   * @return true on success, false otherwise.
   */
  // NOLINTNEXTLINE(modernize-use-nodiscard): Can not check result in Release.
  bool BlitPattern(const RECT& rc,
                   unsigned long raster_operation) const noexcept {
    G3DCHECK(!!device_context_);

    const bool is_succeeded{::PatBlt(device_context_, rc.left, rc.top,
                                     rc.right - rc.left, rc.bottom - rc.top,
                                     raster_operation) != FALSE};
    G3DCHECK(is_succeeded);

    return is_succeeded;
  }

  /**
   * @brief Paint information.
   * @return PAINTSTRUCT.
   */
  [[nodiscard]] const PAINTSTRUCT& PaintInfo() const noexcept {
    return paint_struct_;
  }

  /**
   * @brief Is window device context creation succeeded?
   * @return true if succeeded, false otherwise.
   */
  [[nodiscard]] bool is_succeeded() const noexcept {
    return device_context_ != nullptr;
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

[[nodiscard]] base::std2::result<ScopedWindowPaint> ScopedWindowPaint::New(
    _In_ HWND window) noexcept {
  ScopedWindowPaint scoped_window_paint{window};
  return scoped_window_paint.impl_->is_succeeded()
             ? base::std2::result<ScopedWindowPaint>{std::move(
                   scoped_window_paint)}
             : base::std2::result<ScopedWindowPaint>{
                   std::unexpect,
                   base::std2::system_last_error_code(ERROR_DC_NOT_FOUND)};
}

ScopedWindowPaint::ScopedWindowPaint(_In_ HWND window) noexcept
    : impl_{std::make_unique<ScopedWindowPaintImpl>(window)} {}

ScopedWindowPaint::ScopedWindowPaint(ScopedWindowPaint&& p) noexcept
    : impl_{std::move(p.impl_)} {}

ScopedWindowPaint::~ScopedWindowPaint() = default;

int ScopedWindowPaint::TextDraw(const char* text, int size, RECT* rc,
                                unsigned format) const noexcept {
  return impl_->TextDraw(text, size, rc, format);
}

bool ScopedWindowPaint::BlitPattern(
    const RECT& rc, unsigned long raster_operation) const noexcept {
  return impl_->BlitPattern(rc, raster_operation);
}

[[nodiscard]] const PAINTSTRUCT& ScopedWindowPaint::PaintInfo() const noexcept {
  return impl_->PaintInfo();
}

}  // namespace wb::ui::win
