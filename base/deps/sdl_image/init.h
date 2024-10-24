// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// SDL_image init/quit wrapper.

#ifndef WB_BASE_DEPS_SDL_IMAGE_INIT_H_
#define WB_BASE_DEPS_SDL_IMAGE_INIT_H_

#include <cstddef>  // std::byte
#include <ostream>

#include "base/deps/fmt/format.h"
#include "base/deps/g3log/g3log.h"
#include "base/deps/sdl/base.h"
#include "base/deps/sdl_image/config.h"
#include "base/macroses.h"

WB_BEGIN_SDL_IMAGE_WARNING_OVERRIDE_SCOPE()
#include "deps/sdl_image/include/SDL3_image/SDL_image.h"
WB_END_SDL_IMAGE_WARNING_OVERRIDE_SCOPE()

namespace wb::sdl_image {

/**
 * SDL image initializer flags.
 */
enum class SDLImageInitType : IMG_InitFlags;
}  // namespace wb::sdl_image

/**
 * operator << for SDLImageInitType.
 * @param s Stream.
 * @param flags Flags.
 * @return Stream with dumped flags.
 */
inline std::basic_ostream<char, std::char_traits<char>> &operator<<(
    std::basic_ostream<char, std::char_traits<char>> &s,
    wb::sdl_image::SDLImageInitType flags);

namespace wb::sdl_image {

/**
 * SDL image initializer flags.
 */
enum class SDLImageInitType : IMG_InitFlags {
  kNone = 0x0,
  kJpg = IMG_INIT_JPG,
  kPng = IMG_INIT_PNG,
  kTif = IMG_INIT_TIF,
  kWebp = IMG_INIT_WEBP
};

/**
 * operator | for SDLImageInitType.
 * @param left SDLImageInitType
 * @param right SDLImageInitType.
 * @return Left | Right.
 */
[[nodiscard]] constexpr SDLImageInitType operator|(
    SDLImageInitType left, SDLImageInitType right) noexcept {
  return static_cast<SDLImageInitType>(base::underlying_cast(left) |
                                       base::underlying_cast(right));
}

/**
 * @brief operator & for SDLImageInitType.
 * @param left SDLImageInitType.
 * @param right SDLImageInitType.
 * @return left & right.
 */
[[nodiscard]] constexpr SDLImageInitType operator&(
    SDLImageInitType left, SDLImageInitType right) noexcept {
  return static_cast<SDLImageInitType>(base::underlying_cast(left) &
                                       base::underlying_cast(right));
}

/**
 * SDL image initializer.
 */
class SDLImageInit {
 public:
  /**
   * Creates SDL image initializer.
   * @param flags SDLImageInitType.
   * @return SDL image initializer.
   */
  static sdl::result<SDLImageInit> New(SDLImageInitType flags) noexcept {
    SDLImageInit initializer{flags};
    return initializer.error_code().is_succeeded()
               ? sdl::result<SDLImageInit>{std::move(initializer)}
               : sdl::result<SDLImageInit>{initializer.error_code()};
  }
  SDLImageInit(SDLImageInit &&s) noexcept
      : actual_flags_{s.actual_flags_}, init_rc_{s.init_rc_} {
    s.actual_flags_ = base::underlying_cast(SDLImageInitType::kNone);
    s.init_rc_ = sdl::error::Failure(EINVAL, "Moved");
  }
  SDLImageInit &operator=(SDLImageInit &&) noexcept = delete;

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(SDLImageInit);

  ~SDLImageInit() noexcept { ::IMG_Quit(); }

 private:
  /**
   * Actual returned by SDl flags.
   */
  IMG_InitFlags actual_flags_;
  WB_ATTRIBUTE_UNUSED_FIELD std::byte
      pad_[sizeof(char *) - sizeof(actual_flags_)];  //-V730_NOINIT
  /**
   * Init error code.
   */
  sdl::error init_rc_;

  /**
   * @brief Creates SDL image initializer.
   * @param flags SDLImageInitType.
   */
  explicit SDLImageInit(SDLImageInitType flags) noexcept
      : actual_flags_{::IMG_Init(base::underlying_cast(flags))},
        init_rc_{Initialize(flags, actual_flags_)} {
    G3DCHECK(init_rc_.is_succeeded())
        << "IMG_Init(" << flags << ") failed: " << init_rc_;
  }

  /**
   * Gets error code from flags.
   * @param expected_flags Expected flags.
   * @param actual_flags Actual flags.
   * @return Error code.
   */
  [[nodiscard]] static sdl::error Initialize(SDLImageInitType expected_flags,
                                             IMG_InitFlags actual_flags) noexcept {
    const IMG_InitFlags ex_flags{base::underlying_cast(expected_flags)};

    if ((ex_flags & actual_flags) == ex_flags) {
      return sdl::error::Success();
    }

    return sdl::error::Failure(EINVAL,
                               "Unable to initialize requested image types.");
  }

  /**
   * @brief Init result.
   * @return SDL error.
   */
  [[nodiscard]] sdl::error error_code() const noexcept { return init_rc_; }
};

}  // namespace wb::sdl_image

/**
 * operator << for SDLImageInitType.
 * @param s Stream.
 * @param flags Flags.
 * @return Stream with dumped flags.
 */
inline std::basic_ostream<char, std::char_traits<char>> &operator<<(
    std::basic_ostream<char, std::char_traits<char>> &s,
    wb::sdl_image::SDLImageInitType flags) {
  if (flags == wb::sdl_image::SDLImageInitType::kNone) [[unlikely]] {
    return s << "None";
  }

  if ((flags & wb::sdl_image::SDLImageInitType::kJpg) ==
      wb::sdl_image::SDLImageInitType::kJpg) {
    s << "JPEG ";
  }

  if ((flags & wb::sdl_image::SDLImageInitType::kPng) ==
      wb::sdl_image::SDLImageInitType::kPng) {
    s << "PNG ";
  }

  if ((flags & wb::sdl_image::SDLImageInitType::kTif) ==
      wb::sdl_image::SDLImageInitType::kTif) {
    s << "TIFF ";
  }

  if ((flags & wb::sdl_image::SDLImageInitType::kWebp) ==
      wb::sdl_image::SDLImageInitType::kWebp) {
    s << "WEBP ";
  }

  // (flags & wb::sdl_image::SDLImageInitType::kTif) ==
  // wb::sdl_image::SDLImageInitType::kTif is always false,
  // This is used to prevent compiler from complaining about constant in if
  // condition.
  G3DCHECK((flags & wb::sdl_image::SDLImageInitType::kTif) ==
           wb::sdl_image::SDLImageInitType::kTif)  //-V547
      << "Unknown image type (" << wb::base::underlying_cast(flags)
      << "), assume Unknown.";

  return s << "UNKNOWN ";
}

FMT_BEGIN_NAMESPACE
/**
 * @brief Fmt formatter for wb::sdl_image::SDLImageInitType.
 */
template <>
struct formatter<wb::sdl_image::SDLImageInitType> : formatter<std::string> {
  template <typename FormatContext>
  auto format(wb::sdl_image::SDLImageInitType type, FormatContext &ctx) const {
    std::stringstream s{std::ios_base::out};
    s << type;
    return fmt::formatter<std::string>::format(s.str(), ctx);
  }
};
FMT_END_NAMESPACE

#endif  // !WB_BASE_DEPS_SDL_IMAGE_INIT_H_
