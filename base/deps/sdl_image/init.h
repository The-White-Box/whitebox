// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// SDL_image init/quit wrapper.

#ifndef WB_BASE_DEPS_SDL_IMAGE_INIT_H_
#define WB_BASE_DEPS_SDL_IMAGE_INIT_H_

#include <cstddef>  // std::byte
#include <ostream>

#include "base/base_macroses.h"
#include "base/deps/g3log/g3log.h"
#include "base/deps/sdl/base.h"
#include "deps/sdl_image/SDL_image.h"

namespace wb::sdl_image {
/**
 * SDL image initializer flags.
 */
enum class SdlImageInitializerFlags : int;
}  // namespace wb::sdl_image

/**
 * operator << for SdlImageInitializerFlags.
 * @param s Stream.
 * @param flags Flags.
 * @return Stream with dumped flags.
 */
inline std::basic_ostream<char, std::char_traits<char>> &operator<<(
    std::basic_ostream<char, std::char_traits<char>> &s,
    wb::sdl_image::SdlImageInitializerFlags flags);

namespace wb::sdl_image {
/**
 * SDL image initializer flags.
 */
enum class SdlImageInitializerFlags : int {
  kNone = 0x0,
  kJpg = IMG_INIT_JPG,
  kPng = IMG_INIT_PNG,
  kTif = IMG_INIT_TIF,
  kWebp = IMG_INIT_WEBP
};

/**
 * operator | for SdlImageInitializerFlags.
 * @param left SdlImageInitializerFlags
 * @param right SdlImageInitializerFlags.
 * @return Left | Right.
 */
[[nodiscard]] constexpr SdlImageInitializerFlags operator|(
    SdlImageInitializerFlags left, SdlImageInitializerFlags right) noexcept {
  return static_cast<SdlImageInitializerFlags>(base::underlying_cast(left) |
                                               base::underlying_cast(right));
}

/**
 * @brief operator & for SdlImageInitializerFlags.
 * @param left SdlImageInitializerFlags.
 * @param right SdlImageInitializerFlags.
 * @return left & right.
 */
[[nodiscard]] constexpr SdlImageInitializerFlags operator&(
    SdlImageInitializerFlags left, SdlImageInitializerFlags right) noexcept {
  return static_cast<SdlImageInitializerFlags>(base::underlying_cast(left) &
                                               base::underlying_cast(right));
}

/**
 * SDL image initializer.
 */
class SdlImageInitializer {
 public:
  /**
   * Creates SDL image initializer.
   * @param flags SdlImageInitializerFlags.
   * @return SDL image initializer.
   */
  static sdl::SdlResult<SdlImageInitializer> New(
      SdlImageInitializerFlags flags) noexcept {
    SdlImageInitializer initializer{flags};
    return initializer.error_code().IsSucceeded()
               ? sdl::SdlResult<SdlImageInitializer>{std::move(initializer)}
               : sdl::SdlResult<SdlImageInitializer>{initializer.error_code()};
  }
  SdlImageInitializer(SdlImageInitializer &&s) noexcept
      : actual_flags_{s.actual_flags_}, init_rc_{s.init_rc_} {
    s.actual_flags_ = base::underlying_cast(SdlImageInitializerFlags::kNone);
    s.init_rc_ = sdl::SdlError::Failure("Moved");
  }
  SdlImageInitializer &operator=(SdlImageInitializer &&) noexcept = delete;

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(SdlImageInitializer);

  ~SdlImageInitializer() noexcept { ::IMG_Quit(); }

 private:
  /**
   * Actual returned by SDl flags.
   */
  int actual_flags_;
  std::byte pad_[sizeof(char *) - sizeof(actual_flags_)];
  /**
   * Init error code.
   */
  sdl::SdlError init_rc_;

  /**
   * @brief Creates SDL image initializer.
   * @param flags SdlImageInitializerFlags.
   */
  explicit SdlImageInitializer(SdlImageInitializerFlags flags) noexcept
      : actual_flags_{::IMG_Init(base::underlying_cast(flags))},
        init_rc_{Initialize(flags, actual_flags_)} {
    G3DCHECK(init_rc_.IsSucceeded())
        << "IMG_Init(" << flags << ") failed: " << init_rc_;
  }

  /**
   * Gets error code from flags.
   * @param expected_flags Expected flags.
   * @param actual_flags Actual flags.
   * @return Error code.
   */
  [[nodiscard]] static sdl::SdlError Initialize(
      SdlImageInitializerFlags expected_flags, int actual_flags) noexcept {
    const int ex_flags{base::underlying_cast(expected_flags)};

    if ((ex_flags & actual_flags) == ex_flags) {
      return sdl::SdlError::Success();
    }

    return sdl::SdlError::Failure(
        "Unable to initialize requested image types.");
  }

  /**
   * @brief Init result.
   * @return SDL error.
   */
  [[nodiscard]] sdl::SdlError error_code() const noexcept { return init_rc_; }
};
}  // namespace wb::sdl_image

/**
 * operator << for SdlImageInitializerFlags.
 * @param s Stream.
 * @param flags Flags.
 * @return Stream with dumped flags.
 */
inline std::basic_ostream<char, std::char_traits<char>> &operator<<(
    std::basic_ostream<char, std::char_traits<char>> &s,
    wb::sdl_image::SdlImageInitializerFlags flags) {
  if (flags == wb::sdl_image::SdlImageInitializerFlags::kNone) [[unlikely]] {
    return s << "None";
  }

  if ((flags & wb::sdl_image::SdlImageInitializerFlags::kJpg) ==
      wb::sdl_image::SdlImageInitializerFlags::kJpg) {
    s << "JPEG ";
  }

  if ((flags & wb::sdl_image::SdlImageInitializerFlags::kPng) ==
      wb::sdl_image::SdlImageInitializerFlags::kPng) {
    s << "PNG ";
  }

  if ((flags & wb::sdl_image::SdlImageInitializerFlags::kTif) ==
      wb::sdl_image::SdlImageInitializerFlags::kTif) {
    s << "TIFF ";
  }

  if ((flags & wb::sdl_image::SdlImageInitializerFlags::kWebp) ==
      wb::sdl_image::SdlImageInitializerFlags::kWebp) {
    s << "WEBP ";
  }

  return s;
}

#endif  // !WB_BASE_DEPS_SDL_IMAGE_INIT_H_
