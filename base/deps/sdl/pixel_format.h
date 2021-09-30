// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// SDL_pixels.h wrapper.

#ifndef WHITEBOX_BASE_DEPS_SDL_PIXEL_FORMAT_H_
#define WHITEBOX_BASE_DEPS_SDL_PIXEL_FORMAT_H_

#include <ostream>

#include "base/base_macroses.h"
#include "base/deps/g3log/g3log.h"
#include "base/deps/sdl/base.h"
#include "base/deps/sdl/config.h"
//
WB_BEGIN_SDL_WARNING_OVERRIDE_SCOPE()
#include "deps/sdl/include/SDL_pixels.h"
WB_END_SDL_WARNING_OVERRIDE_SCOPE()

namespace wb::sdl {
/**
 * SDL pixel format.
 */
enum class SdlPixelFormatFlag : Uint32 {};

/**
 * SDL pixel format.
 */
class SdlPixelFormat {
  friend class SdlSurface;

 public:
  /**
   * Creates new pixel format.
   * @param value SDL pixel format value.
   * @return SDL pixel format.
   */
  [[nodiscard]] static SdlResult<SdlPixelFormat> New(
      SdlPixelFormatFlag value) noexcept {
    SdlPixelFormat format{value};
    return format.error_code().is_succeeded()
               ? SdlResult<SdlPixelFormat>{std::move(format)}
               : SdlResult<SdlPixelFormat>{format.error_code()};
  }
  
  SdlPixelFormat(SdlPixelFormat &&f) noexcept : format_{f.format_}, init_rc_{f.init_rc_} {
    f.format_ = nullptr;
  }
  SdlPixelFormat& operator=(SdlPixelFormat &&f) noexcept = delete;

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(SdlPixelFormat);

  ~SdlPixelFormat() noexcept {
    if (format_) {
      ::SDL_FreeFormat(format_);
      format_ = nullptr;
    }
  }

 private:
  /**
   * SDL pixel format.
   */
  SDL_PixelFormat *format_;
  /**
   * SDL init result.
   */
  SdlError init_rc_;

  /**
   * Creates SDL pixel format.
   * @param value Pixel format value.
   */
  explicit SdlPixelFormat(SdlPixelFormatFlag value) noexcept
      : format_{::SDL_AllocFormat(base::underlying_cast(value))},
        init_rc_{format_ ? SdlError::Success() : SdlError::Failure()} {}

  /**
   * @brief Init result.
   * @return SDL error.
   */
  [[nodiscard]] SdlError error_code() const noexcept { return init_rc_; }
};
}  // namespace wb::sdl

#endif  // !WHITEBOX_BASE_DEPS_SDL_PIXEL_FORMAT_H_
