// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// SDL_pixels.h wrapper.

#ifndef WHITEBOX_BASE_DEPS_SDL_PIXEL_FORMAT_H_
#define WHITEBOX_BASE_DEPS_SDL_PIXEL_FORMAT_H_

#include <ostream>

#include "base/deps/g3log/g3log.h"
#include "base/deps/sdl/base.h"
#include "base/deps/sdl/config.h"
#include "base/macroses.h"
//
WB_BEGIN_SDL_WARNING_OVERRIDE_SCOPE()
#include "deps/sdl/include/SDL3/SDL_pixels.h"
WB_END_SDL_WARNING_OVERRIDE_SCOPE()

namespace wb::sdl {

/**
 * SDL pixel format.
 */
enum class PixelFormatFlag : Uint32 {};

/**
 * SDL pixel format.
 */
class PixelFormat {
  friend class Surface;

 public:
  /**
   * Creates new pixel format.
   * @param value SDL pixel format value.
   * @return SDL pixel format.
   */
  [[nodiscard]] static result<PixelFormat> New(PixelFormatFlag value) noexcept {
    PixelFormat format{value};
    return format.error_code().is_succeeded()
               ? result<PixelFormat>{std::move(format)}
               : result<PixelFormat>{std::unexpect, format.error_code()};
  }

  PixelFormat(PixelFormat &&f) noexcept
      : format_{f.format_}, init_rc_{f.init_rc_} {
    f.format_ = nullptr;
  }
  PixelFormat &operator=(PixelFormat &&f) noexcept = delete;

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(PixelFormat);

  ~PixelFormat() noexcept {
    if (format_) {
      // Format is stored in hash table inside SDL, can't free it easily.
      format_ = nullptr;
    }
  }

 private:
  /**
   * SDL pixel format.
   */
  const SDL_PixelFormatDetails *format_;
  /**
   * SDL init result.
   */
  error init_rc_;

  /**
   * Creates SDL pixel format.
   * @param value Pixel format value.
   */
  explicit PixelFormat(PixelFormatFlag value) noexcept
      : format_{::SDL_GetPixelFormatDetails(
            static_cast<SDL_PixelFormat>(base::underlying_cast(value)))},
        init_rc_{format_ ? error::Success() : error::Failure()} {}

  /**
   * @brief Init result.
   * @return SDL error.
   */
  [[nodiscard]] error error_code() const noexcept { return init_rc_; }
};

}  // namespace wb::sdl

#endif  // !WHITEBOX_BASE_DEPS_SDL_PIXEL_FORMAT_H_
