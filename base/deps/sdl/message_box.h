// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// SDL message box wrapper.

#ifndef WB_BASE_DEPS_SDL_MESSAGE_BOX_H_
#define WB_BASE_DEPS_SDL_MESSAGE_BOX_H_

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

/**
 * @brief Fatal error string stream.  Dumps error to log, shows UI and exit.
 */
class FatalErrorStringStream final : public std::stringstream {
 public:
  /**
   * Creates fatal error string stream.
   * @param title Title of error message box.
   * @param rc Error code.
   */
  FatalErrorStringStream(std::string title, std::error_code rc) noexcept
      : std::stringstream{std::ios::out}, title_{std::move(title)}, rc_{rc} {}
  /**
   * Creates fatal error string stream.
   * @param title Title of error message box.
   */
  explicit FatalErrorStringStream(std::string title) noexcept
      : FatalErrorStringStream{std::move(title), std::error_code{}} {}

  FatalErrorStringStream(FatalErrorStringStream&& s) noexcept
      : std::stringstream{std::move(s)}, title_{std::move(s.title_)} {}
  FatalErrorStringStream& operator=(FatalErrorStringStream&&) = delete;

  [[noreturn]] ~FatalErrorStringStream() noexcept override {
    const std::string error{str()};

    if (rc_) {
      G3PLOG_E(WARNING, rc_) << error;
    } else {
      G3LOG(WARNING) << error;
    }

    ShowSimpleMessageBox(MessageBoxFlags::Error | MessageBoxFlags::LeftToRight,
                         title_.c_str(), error.c_str());

    std::exit(rc_ ? rc_.value() : 1);
  }

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(FatalErrorStringStream);

 private:
  std::string title_;
  std::error_code rc_;
};

/**
 * Creates fatal error stream, which dumps error to log, shows UI message box
 * and exits.
 * @param title Message box title.
 * @return FatalErrorStringStream.
 */
[[nodiscard]] inline FatalErrorStringStream Fatal(std::string title) {
  return FatalErrorStringStream{std::move(title)};
}

/**
 * Creates fatal error stream, which dumps error + error code to log, shows UI
 * message box and exits.
 * @param title Message box title.
 * @param rc Error code.
 * @return FatalErrorStringStream.
 */
[[nodiscard]] inline FatalErrorStringStream Fatal(std::string title,
                                                  std::error_code rc) {
  return FatalErrorStringStream{std::move(title), rc};
}
}  // namespace wb::sdl

#endif  // !WB_BASE_DEPS_SDL_MESSAGE_BOX_H_
