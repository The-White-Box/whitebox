// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// SDL message box wrapper.

#ifndef WB_BASE_DEPS_SDL_MESSAGE_BOX_H_
#define WB_BASE_DEPS_SDL_MESSAGE_BOX_H_

#include <sstream>

#include "base/deps/g3log/g3log.h"
#include "base/deps/sdl/base.h"
#include "base/deps/sdl/sdl.h"

namespace wb::sdl {

/**
 * Message box flags.  See SDL_MESSAGEBOX_* for details.
 */
enum class MessageBoxFlags : Uint32 {
  Information = SDL_MESSAGEBOX_INFORMATION,
  Warning = SDL_MESSAGEBOX_WARNING,
  Error = SDL_MESSAGEBOX_ERROR,
  LeftToRight = SDL_MESSAGEBOX_BUTTONS_LEFT_TO_RIGHT,
  RightToLeft = SDL_MESSAGEBOX_BUTTONS_RIGHT_TO_LEFT
};

/**
 * @brief operator | for MessageBoxFlags.
 * @param left MessageBoxFlags.
 * @param right MessageBoxFlags.
 * @return left | right.
 */
[[nodiscard]] constexpr MessageBoxFlags operator|(
    MessageBoxFlags left, MessageBoxFlags right) noexcept {
  return static_cast<MessageBoxFlags>(base::underlying_cast(left) |
                                      base::underlying_cast(right));
}

/**
 * Show simple message box.
 * @param flags MessageBoxFlags.
 * @param title Title.
 * @param message Message.
 * @return SDL error.
 */
inline SdlError ShowSimpleMessageBox(MessageBoxFlags flags, const char* title,
                                     const char* message) noexcept {
  return SdlError::FromReturnCode(::SDL_ShowSimpleMessageBox(
      base::underlying_cast(flags), title, message, nullptr));
}

}  // namespace wb::sdl

#endif  // !WB_BASE_DEPS_SDL_MESSAGE_BOX_H_
