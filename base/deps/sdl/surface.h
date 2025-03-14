// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// SDL_surface.h wrapper.

#ifndef WHITEBOX_BASE_DEPS_SDL_SURFACE_H_
#define WHITEBOX_BASE_DEPS_SDL_SURFACE_H_

#include "base/deps/g3log/g3log.h"
#include "base/deps/sdl/base.h"
#include "base/deps/sdl/config.h"
#include "base/deps/sdl/pixel_format.h"
#include "base/deps/sdl_image/sdl_image.h"
#include "base/macroses.h"
//
WB_BEGIN_SDL_WARNING_OVERRIDE_SCOPE()
#include "deps/sdl/include/SDL3/SDL_surface.h"
WB_END_SDL_WARNING_OVERRIDE_SCOPE()

namespace wb::sdl {

/**
 * SDL surface mask.
 */
struct SurfaceMask {
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
class Surface {
  friend class ScopedSurfaceLock;
  friend class Window;

 public:
  /**
   * Creates surface from RGB data.
   * @param width Width.
   * @param height Height.
   * @param depth Depth.
   * @param mask RGB mask.
   * @return SDL surface.
   */
  [[nodiscard]] static result<Surface> FromRgb(
      int width, int height, int depth, const SurfaceMask &mask) noexcept {
    Surface surface{width, height, depth, mask};
    return surface.error_code().is_succeeded()
               ? result<Surface>{std::move(surface)}
               : result<Surface>{std::unexpect, surface.error_code()};
  }

  /**
   * Creates surface from image data.  Supported BMP, PNM (PPM/PGM/PBM), XPM,
   * LBM, PCX, GIF, JPEG, PNG, TGA, TIFF, and simple SVG formats.
   * @param image_path Image file path.
   * @return SDL surface.
   */
  [[nodiscard]] static result<Surface> FromImage(
      const char *image_path) noexcept {
    Surface surface{image_path};
    return surface.error_code().is_succeeded()
               ? result<Surface>{std::move(surface)}
               : result<Surface>{std::unexpect, surface.error_code()};
  }

  /**
   * Creates surface from another surface + pixel format.
   * @param source Source surface.
   * @param format Pixel format.
   * @return SDL surface.
   */
  [[nodiscard]] static result<Surface> FromSurface(
      Surface &source, const PixelFormat &format) noexcept {
    Surface surface{source, format};
    return surface.error_code().is_succeeded()
               ? result<Surface>{std::move(surface)}
               : result<Surface>{std::unexpect, surface.error_code()};
  }

  Surface(Surface &&s) noexcept : surface_{s.surface_}, init_rc_{s.init_rc_} {
    s.surface_ = nullptr;
    s.init_rc_ = error::Failure();
  }

  Surface &operator=(Surface &&) = delete;

  ~Surface() noexcept {
    if (surface_) {
      ::SDL_DestroySurface(surface_);
      surface_ = nullptr;
    }
  }

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(Surface);

 private:
  /**
   * SDL surface.
   */
  SDL_Surface *surface_;
  /**
   * SDL surface init result.
   */
  error init_rc_;

  /**
   * Creates surface from RGB data.
   * @param width Width.
   * @param height Height.
   * @param depth Depth.
   * @param mask RGB mask.
   * @return nothing.
   */
  Surface(int width, int height, int depth, const SurfaceMask &mask) noexcept
      : surface_{::SDL_CreateSurface(
            width, height,
            SDL_GetPixelFormatForMasks(depth, mask.red, mask.green, mask.blue,
                                       mask.alpha))},
        init_rc_{surface_ ? error::Success() : error::Failure()} {
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
  explicit Surface(const char *image_path) noexcept
      : surface_{::IMG_Load(image_path)},
        init_rc_{surface_ ? error::Success() : error::Failure()} {
    G3DCHECK(!!surface_) << "IMG_Load(" << image_path
                         << ") failed with error: " << error_code();
  }

  /**
   * Creates surface from another surface + pixel format.
   * @param source Source surface.
   * @param format Pixel format.
   * @return nothing.
   */
  Surface(Surface &source, const PixelFormat &format) noexcept
      : surface_{::SDL_ConvertSurface(source.surface_, format.format_->format)},
        init_rc_{surface_ ? error::Success() : error::Failure()} {
    G3DCHECK(!!surface_) << "SDL_ConvertSurface(" << format.format_
                         << ") failed with error: " << error_code();
  }

  /**
   * @brief Init result.
   * @return SDL error.
   */
  [[nodiscard]] error error_code() const noexcept { return init_rc_; }
};

/**
 * Locks surface in scope.
 */
class ScopedSurfaceLock {
 public:
  /**
   * Creates scoped surface lock.
   * @param surface Surface to lock.
   */
  explicit ScopedSurfaceLock(Surface &surface) noexcept
      : surface_{surface.surface_} {
    G3DCHECK(!!surface_);

    if (SDL_MUSTLOCK(surface_)) {
      const int rc{::SDL_LockSurface(surface_)};
      G3CHECK(rc == 0) << "SDL surface lock failed w/e "
                       << error::FromReturnCode(rc);
    }
  }
  ~ScopedSurfaceLock() noexcept {
    G3DCHECK(!!surface_);

    if (SDL_MUSTLOCK(surface_)) {
      ::SDL_UnlockSurface(surface_);
    }
    surface_ = nullptr;
  }

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedSurfaceLock);

 private:
  /**
   * SDL surface.
   */
  SDL_Surface *surface_;
};

}  // namespace wb::sdl

#endif  // !WHITEBOX_BASE_DEPS_SDL_SURFACE_H_
