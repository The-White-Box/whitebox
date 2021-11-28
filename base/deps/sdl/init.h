// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// SDL init/quit wrapper.

#ifndef WB_BASE_DEPS_SDL_INIT_H_
#define WB_BASE_DEPS_SDL_INIT_H_

#include <array>
#include <cstddef>

#include "base/macroses.h"
#include "base/deps/sdl/base.h"
#include "base/deps/sdl/sdl.h"
#include "base/std2/cstring_ext.h"
#include "base/std2/system_error_ext.h"

namespace wb::sdl {

/**
 * @brief SDL initializer flags.  See SDL_INIT_* fro details.
 */
enum class SDLInitializerFlags : Uint32 {
  kNone = 0U,
  kTimer = SDL_INIT_TIMER,
  kAudio = SDL_INIT_AUDIO,
  kVideo = SDL_INIT_VIDEO,
  kJoystick = SDL_INIT_JOYSTICK,
  kHaptic = SDL_INIT_HAPTIC,
  kGameController = SDL_INIT_GAMECONTROLLER,
  kEvents = SDL_INIT_EVENTS,
  kSensor = SDL_INIT_SENSOR,
  KNoParachute = SDL_INIT_NOPARACHUTE,
  KEverything = SDL_INIT_EVERYTHING
};

/**
 * @brief operator | for SDLInitializerFlags.
 * @param left SDLInitializerFlags.
 * @param right SDLInitializerFlags.
 * @return left | right.
 */
[[nodiscard]] constexpr SDLInitializerFlags operator|(
    SDLInitializerFlags left, SDLInitializerFlags right) noexcept {
  return static_cast<SDLInitializerFlags>(base::underlying_cast(left) |
                                          base::underlying_cast(right));
}

/**
 * SDL initializer.
 */
class SDLInitializer {
 public:
  /**
   * Creates SDL initializer.
   * @param flags SDLInitializerFlags.
   * @return SDL initializer.
   */
  static result<SDLInitializer> New(SDLInitializerFlags flags) noexcept {
    SDLInitializer initializer{flags};
    return initializer.error_code().is_succeeded()
               ? result<SDLInitializer>{std::move(initializer)}
               : result<SDLInitializer>{initializer.error_code()};
  }
  /**
   * Move constructor.
   * @param s Type to move frpm.
   */
  SDLInitializer(SDLInitializer &&s) noexcept
      : init_rc_{s.init_rc_}, flags_{s.flags_} {
    s.init_rc_ = error::Failure();
    s.flags_ = SDLInitializerFlags::kNone;
  }
  SDLInitializer &operator=(SDLInitializer &&) noexcept = delete;

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(SDLInitializer);

  ~SDLInitializer() noexcept {
    if (init_rc_.is_succeeded()) {
      ::SDL_Quit();
    }
  }

 private:
  /**
   * SDL init return code.
   */
  error init_rc_;
  /**
   * SDL initializer flags.
   */
  SDLInitializerFlags flags_;

  WB_ATTRIBUTE_UNUSED_FIELD
  std::array<std::byte, sizeof(char *) - sizeof(flags_)> pad_;

  /**
   * @brief Creates SDL initializer.
   * @param flags SDLInitializerFlags.
   */
  explicit SDLInitializer(SDLInitializerFlags flags) noexcept
      : init_rc_{error::FromReturnCode(
            ::SDL_Init(wb::base::underlying_cast(flags)))},
        flags_{flags} {}

  /**
   * Init result.
   * @return SDL error.
   */
  [[nodiscard]] error error_code() const noexcept { return init_rc_; }
};

}  // namespace wb::sdl

#endif  // !WB_BASE_DEPS_SDL_INIT_H_
