// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// SDL init/quit wrapper.

#ifndef WB_BASE_DEPS_SDL_INIT_H_
#define WB_BASE_DEPS_SDL_INIT_H_

#include "base/base_macroses.h"
#include "base/deps/sdl/base.h"
#include "base/deps/sdl/sdl.h"
#include "base/std2/cstring_ext.h"
#include "base/std2/system_error_ext.h"

namespace wb::sdl {

/**
 * @brief SDL initializer flags.  See SDL_INIT_* fro details.
 */
enum class SdlInitializerFlags : Uint32 {
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
 * @brief operator | for SdlInitializerFlags.
 * @param left SdlInitializerFlags.
 * @param right SdlInitializerFlags.
 * @return left | right.
 */
[[nodiscard]] constexpr SdlInitializerFlags operator|(
    SdlInitializerFlags left, SdlInitializerFlags right) noexcept {
  return static_cast<SdlInitializerFlags>(base::underlying_cast(left) |
                                          base::underlying_cast(right));
}

/**
 * SDL initializer.
 */
class SdlInitializer {
 public:
  /**
   * Creates SDL initializer.
   * @param flags SdlInitializerFlags.
   * @return SDL initializer.
   */
  static SdlResult<SdlInitializer> New(SdlInitializerFlags flags) noexcept {
    SdlInitializer initializer{flags};
    return initializer.error_code().is_succeeded()
               ? SdlResult<SdlInitializer>{std::move(initializer)}
               : SdlResult<SdlInitializer>{initializer.error_code()};
  }
  SdlInitializer(SdlInitializer &&s) noexcept
      : init_rc_{s.init_rc_}, flags_{s.flags_} {
    s.init_rc_ = SdlError::Failure();
    s.flags_ = SdlInitializerFlags::kNone;
  }
  SdlInitializer &operator=(SdlInitializer &&) noexcept = delete;

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(SdlInitializer);

  ~SdlInitializer() noexcept {
    if (init_rc_.is_succeeded()) {
      ::SDL_Quit();
    }
  }

 private:
  SdlError init_rc_;
  SdlInitializerFlags flags_;
  // NOLINTNEXTLINE(modernize-avoid-c-arrays)
  WB_ATTRIBUTE_UNUSED_FIELD std::byte pad_[sizeof(char *) - sizeof(flags_)];

  /**
   * @brief Creates SDL initializer.
   * @param flags SdlInitializerFlags.
   */
  explicit SdlInitializer(SdlInitializerFlags flags) noexcept
      : init_rc_{SdlError::FromReturnCode(
            ::SDL_Init(wb::base::underlying_cast(flags)))},
        flags_{flags} {}

  /**
   * Init result.
   * @return SDL error.
   */
  [[nodiscard]] SdlError error_code() const noexcept { return init_rc_; }
};

}  // namespace wb::sdl

#endif  // !WB_BASE_DEPS_SDL_INIT_H_
