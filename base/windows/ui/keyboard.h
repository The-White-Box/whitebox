// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Keyboard input device.

#ifndef WB_BASE_WINDOWS_UI_KEYBOARD_H_
#define WB_BASE_WINDOWS_UI_KEYBOARD_H_

#include <string>
#include <system_error>

#include "base/base_api.h"
#include "base/base_macroses.h"

using HWND = struct HWND__ *;
using RAWINPUT = struct tagRAWINPUT;

namespace wb::base::windows::ui {
/**
 * @brief Flags for scan code information.
 */
enum class KeyboardKeyFlags : unsigned short {
  /**
   * @brief The key is down.  Note default Windows code is 0, but we remap to
   * this as it is hard to check via bitwise & when ex E0 prefix also used.
   */
  kDown = 0x8000U,
  /**
   * @brief The key is up.
   */
  kUp = 0x0001U,
  /**
   * @brief The scan code has the E0 prefix.  The codes E0 and E1 introduce scan
   * code sequences.  Ex. Logitech uses an E2 prefix for the codes sent by a
   * pointing device integrated on the keyboard.
   */
  kE0Prefix = 0x0002U,
  /**
   * @brief The scan code has the E1 prefix.  The codes E0 and E1 introduce scan
   * code sequences.  Ex. Logitech uses an E2 prefix for the codes sent by a
   * pointing device integrated on the keyboard.
   */
  kE1Prefix = 0x0004U,
  /**
   * @brief Terminal server enable led?
   */
  kTerminalServerSetLed = 0x0008U,
  /**
   * @brief Terminal server disable led?
   */
  kTerminalServerShadow = 0x0010U
};

/**
 * @brief operator&.
 * @param left Left.
 * @param right Right
 * @return left & right.
 */
[[nodiscard]] constexpr KeyboardKeyFlags operator&(
    KeyboardKeyFlags left, KeyboardKeyFlags right) noexcept {
  return static_cast<KeyboardKeyFlags>(underlying_cast(left) &
                                       underlying_cast(right));
}

/**
 * @brief operator|.
 * @param left Left.
 * @param right Right
 * @return left | right.
 */
[[nodiscard]] constexpr KeyboardKeyFlags operator|(
    KeyboardKeyFlags left, KeyboardKeyFlags right) noexcept {
  return static_cast<KeyboardKeyFlags>(underlying_cast(left) |
                                       underlying_cast(right));
}

/**
 * @brief String representation of keyboard flags.
 * @param key_flags Flags.
 * @return string.
 */
[[nodiscard]] inline std::string to_string(KeyboardKeyFlags key_flags) {
  std::string result;
  result.reserve(32);

  if ((key_flags & KeyboardKeyFlags::kDown) == KeyboardKeyFlags::kDown) {
    result += "Down ";
  } else if ((key_flags & KeyboardKeyFlags::kUp) == KeyboardKeyFlags::kUp) {
    result += "Up ";
  }

  if ((key_flags & KeyboardKeyFlags::kE0Prefix) ==
      KeyboardKeyFlags::kE0Prefix) {
    result += "E0 ";
  } else if ((key_flags & KeyboardKeyFlags::kE1Prefix) ==
             KeyboardKeyFlags::kE1Prefix) {
    result += "E1 ";
  }

  if ((key_flags & KeyboardKeyFlags::kTerminalServerSetLed) ==
      KeyboardKeyFlags::kTerminalServerSetLed) {
    result += "Terminal On LED ";
  }

  if ((key_flags & KeyboardKeyFlags::kTerminalServerShadow) ==
      KeyboardKeyFlags::kTerminalServerShadow) {
    result += "Terminal Off LED ";
  }

  return result;
};

/**
 * @brief Keyboard input.
 */
struct KeyboardInput {
  /*
   * The "make" scan code (key depression).  May be kKeyboardOverrunMakeCode.
   */
  unsigned short make_code;

  /*
   * The flags field indicates a "break" (key release) and other
   * miscellaneous scan code information defined in ntddkbd.h.
   */
  KeyboardKeyFlags key_flags;

  /**
   * @brief To align to machine word.
   */
  unsigned short reserved;

  /*
   * The corresponding legacy virtual-key code.
   */
  unsigned short virtual_key;

  /*
   * The corresponding legacy keyboard window message.
   */
  unsigned message;

  /**
   * @brief Special MakeCode value sent when an invalid or unrecognizable
   * combination of keys is pressed or the number of keys pressed exceeds the
   * limit for this keyboard.
   */
  static constexpr inline unsigned short kOverrunMakeCode{0xFFU};
};

/**
 * @brief Keyboard input as string.
 * @param keyboard_input Keyboard input.
 * @return string.
 */
[[nodiscard]] inline std::string to_string(
    const KeyboardInput &keyboard_input) {
  std::string result;
  result.reserve(256);

  using std::to_string;

  result += "Make Code: " + to_string(keyboard_input.make_code);
  result += " | Key Flags: " + to_string(keyboard_input.key_flags);
  result += " | Virtual Key: " + to_string(keyboard_input.virtual_key);
  result += " | Message: " + to_string(keyboard_input.message);

  return result;
}

/**
 * @brief Low level keyboard input device.
 */
class Keyboard {
 public:
  /**
   * @brief Creates keyboard device.
   * @param window Window to handle keyboard input.
   * @return nothing.
   */
  WB_BASE_API Keyboard(_In_ HWND window) noexcept;
  /**
   * @brief Shut down keyboard device.
   */
  WB_BASE_API ~Keyboard() noexcept;

  Keyboard(Keyboard &&) = default;
  Keyboard &operator=(Keyboard &&) = default;

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(Keyboard);

  /**
   * @brief Keyboard initialization error code.
   * @return Error code.
   */
  [[nodiscard]] std::error_code error_code() const noexcept {
    return error_code_;
  }

  /**
   * @brief Handle raw input.
   * @param raw_input Raw input.
   * @param keyboard_input Raw input as keyboard input if it is keyboard input.
   * @return true if raw input is keyboard input, false otherwise.
   */
  [[nodiscard]] WB_BASE_API bool Handle(const RAWINPUT &raw_input,
                                        KeyboardInput &keyboard_input) noexcept;

 private:
  /**
   * @brief Window handle to get keyboard input for.
   */
  HWND window_;
  /**
   * @brief Keyboard initialization error code.
   */
  std::error_code error_code_;
};
}  // namespace wb::base::windows::ui

#endif  // !WB_BASE_WINDOWS_UI_KEYBOARD_H_
