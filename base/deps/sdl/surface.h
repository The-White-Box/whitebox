// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// SDL_surface.h wrapper.

#ifndef WHITEBOX_BASE_DEPS_SDL_SURFACE_H_
#define WHITEBOX_BASE_DEPS_SDL_SURFACE_H_

#include "base/base_macroses.h"
#include "base/deps/g3log/g3log.h"
#include "base/deps/sdl/base.h"
#include "base/deps/sdl/config.h"
#include "base/deps/sdl/pixel_format.h"
//
WB_BEGIN_SDL_WARNING_OVERRIDE_SCOPE()
#include "deps/sdl/include/SDL_surface.h"
#include "deps/sdl_image/SDL_image.h"
WB_END_SDL_WARNING_OVERRIDE_SCOPE()

namespace wb::sdl {

/**
 * SDL surface mask.
 */
struct SdlSurfaceMask {
  /**
   * Red channel mask.
   */
  uint32_t red;
  /**
   * Green channel mask.
   */
  uint32_t green;
  /**
   * Blue channel mask.
   */
  uint32_t blue;
  /**
   * Alpha channel mask.
   */
  uint32_t alpha;
};

/**
 * SDL surface.
 */
class SdlSurface {
  friend class ScopedSdlSurfaceLock;
  friend class SdlWindow;

 public:
  /**
   * Creates surface from RGB data.
   * @param width Width.
   * @param height Height.
   * @param depth Depth.
   * @param mask RGB mask.
   * @return SDL surface.
   */
  [[nodiscard]] static SdlResult<SdlSurface> FromRgb(
      int width, int height, int depth, const SdlSurfaceMask &mask) noexcept {
    SdlSurface surface{width, height, depth, mask};
    return surface.error_code().is_succeeded()
               ? SdlResult<SdlSurface>{std::move(surface)}
               : SdlResult<SdlSurface>{surface.error_code()};
  }

  /**
   * Creates surface from image data.  Supported BMP, PNM (PPM/PGM/PBM), XPM,
   * LBM, PCX, GIF, JPEG, PNG, TGA, TIFF, and simple SVG formats.
   * @param image_path Image file path.
   * @return SDL surface.
   */
  [[nodiscard]] static SdlResult<SdlSurface> FromImage(
      const char *image_path) noexcept {
    SdlSurface surface{image_path};
    return surface.error_code().is_succeeded()
               ? SdlResult<SdlSurface>{std::move(surface)}
               : SdlResult<SdlSurface>{surface.error_code()};
  }

  /**
   * Creates surface from another surface + pixel format.
   * @param source Source surface.
   * @param format Pixel format.
   * @return SDL surface.
   */
  [[nodiscard]] static SdlResult<SdlSurface> FromSurface(
      SdlSurface &source, const SdlPixelFormat &format) noexcept {
    SdlSurface surface{source, format};
    return surface.error_code().is_succeeded()
               ? SdlResult<SdlSurface>{std::move(surface)}
               : SdlResult<SdlSurface>{surface.error_code()};
  }

  SdlSurface(SdlSurface &&s) noexcept
      : surface_{s.surface_}, init_rc_{s.init_rc_} {
    s.surface_ = nullptr;
    s.init_rc_ = SdlError::Failure();
  }

  SdlSurface &operator=(SdlSurface &&) = delete;

  ~SdlSurface() noexcept {
    if (surface_) {
      ::SDL_FreeSurface(surface_);
      surface_ = nullptr;
    }
  }

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(SdlSurface);

 private:
  /**
   * SDL surface.
   */
  SDL_Surface *surface_;
  /**
   * SDL surface init result.
   */
  SdlError init_rc_;

  /**
   * Creates surface from RGB data.
   * @param width Width.
   * @param height Height.
   * @param depth Depth.
   * @param mask RGB mask.
   * @return nothing.
   */
  SdlSurface(int width, int height, int depth,
             const SdlSurfaceMask &mask) noexcept
      : surface_{::SDL_CreateRGBSurface(0U, width, height, depth, mask.red,
                                        mask.green, mask.blue, mask.alpha)},
        init_rc_{surface_ ? SdlError::Success() : SdlError::Failure()} {
    G3DCHECK(!!surface_) << "SDL_CreateRGBSurface(" << width << ", " << height
                         << ", " << depth
                         << ") failed with error: " << error_code();
  }

  /**
   * Creates surface from image data.  Supported BMP, PNM (PPM/PGM/PBM), XPM,
   * LBM, PCX, GIF, JPEG, PNG, TGA, TIFF, and simple SVG formats.
   * @param image_path Image file path.
   * @return nothing.
   */
  explicit SdlSurface(const char *image_path) noexcept
      : surface_{::IMG_Load(image_path)},
        init_rc_{surface_ ? SdlError::Success() : SdlError::Failure()} {
    G3DCHECK(!!surface_) << "IMG_Load(" << image_path
                         << ") failed with error: " << error_code();
  }

  /**
   * Creates surface from another surface + pixel format.
   * @param source Source surface.
   * @param format Pixel format.
   * @return nothing.
   */
  SdlSurface(SdlSurface &source, const SdlPixelFormat &format) noexcept
      : surface_{::SDL_ConvertSurface(source.surface_, format.format_, 0U)},
        init_rc_{surface_ ? SdlError::Success() : SdlError::Failure()} {
    G3DCHECK(!!surface_) << "SDL_ConvertSurface(" << format.format_
                         << ") failed with error: " << error_code();
  }

  /**
   * @brief Init result.
   * @return SDL error.
   */
  [[nodiscard]] SdlError error_code() const noexcept { return init_rc_; }
};

/**
 * Locks surface in scope.
 */
class ScopedSdlSurfaceLock {
 public:
  /**
   * Creates scoped surface lock.
   * @param surface Surface to lock.
   */
  explicit ScopedSdlSurfaceLock(SdlSurface &surface) noexcept
      : surface_{surface.surface_} {
    G3DCHECK(!!surface_);

    if (SDL_MUSTLOCK(surface_)) {
      const int rc{::SDL_LockSurface(surface_)};
      G3CHECK(rc == 0) << "SDL surface lock failed w/e "
                       << SdlError::FromReturnCode(rc);
    }
  }
  ~ScopedSdlSurfaceLock() noexcept {
    G3DCHECK(!!surface_);

    if (SDL_MUSTLOCK(surface_)) {
      ::SDL_UnlockSurface(surface_);
    }
    surface_ = nullptr;
  }

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedSdlSurfaceLock);

 private:
  /**
   * SDL surface.
   */
  SDL_Surface *surface_;
};

}  // namespace wb::sdl

#endif  // !WHITEBOX_BASE_DEPS_SDL_SURFACE_H_
