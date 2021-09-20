// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Keyboard input definitions.

#ifndef WB_HAL_DRIVERS_HID_KEYBOARD_INPUT_H_
#define WB_HAL_DRIVERS_HID_KEYBOARD_INPUT_H_

#include <string>

#include "base/base_macroses.h"
#include "build/build_config.h"

namespace wb::hal::hid {
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
  return static_cast<KeyboardKeyFlags>(base::underlying_cast(left) &
                                       base::underlying_cast(right));
}

/**
 * @brief operator|.
 * @param left Left.
 * @param right Right
 * @return left | right.
 */
[[nodiscard]] constexpr KeyboardKeyFlags operator|(
    KeyboardKeyFlags left, KeyboardKeyFlags right) noexcept {
  return static_cast<KeyboardKeyFlags>(base::underlying_cast(left) |
                                       base::underlying_cast(right));
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
}

/**
 * @brief Keyboard input.
 */
struct KeyboardInput {
  /*
   * The "make" scan code (key depression).  May be kOverrunMakeCode.
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

  // TODO(dimhotepus): Map values below and remove entirely?

#ifdef WB_OS_WIN
  /*
   * The corresponding legacy virtual-key code.
   */
  unsigned short virtual_key;

  /*
   * The corresponding legacy keyboard window message.
   */
  unsigned message;
#endif

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

  result += "Make Code: " +
            (keyboard_input.make_code != KeyboardInput::kOverrunMakeCode
                 ? to_string(keyboard_input.make_code)
                 : "Overrun");
  result += " | Key Flags: " + to_string(keyboard_input.key_flags);
  result += " | Virtual Key: " + to_string(keyboard_input.virtual_key);
  result += " | Message: " + to_string(keyboard_input.message);

  return result;
}
}  // namespace wb::hal::hid

#endif  // !WB_HAL_DRIVERS_HID_KEYBOARD_INPUT_H_
