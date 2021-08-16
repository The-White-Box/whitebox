// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Keyboard input device.

#include "keyboard_win.h"

#include "base/deps/g3log/g3log.h"
#include "base/windows/windows_light.h"
#include "raw_input_win.h"
// Should go after windows.h
#include <hidusage.h>

namespace {
/**
 * @brief Create keyboard device definition.
 * @param window Window.
 * @param flags Keyboard device flags.
 * @return Keyboard device definition.
 */
[[nodiscard]] RAWINPUTDEVICE CreateKeyboardDeviceDefinition(
    _In_opt_ HWND window, _In_ unsigned long flags) {
  return RAWINPUTDEVICE{.usUsagePage = HID_USAGE_PAGE_GENERIC,
                        .usUsage = HID_USAGE_GENERIC_KEYBOARD,
                        .dwFlags = flags,
                        .hwndTarget = window};
}
}  // namespace

namespace wb::hal::hid {
/**
 * @brief Alias to simplify API.
 */
using KeyboardNewResult = base::std_ext::os_res<base::un<Keyboard>>;

/**
 * @brief Creates keyboard device.
 * @param window Window to handle keyboard input.
 * @return Keyboard.
 */
[[nodiscard]] KeyboardNewResult Keyboard::New(_In_ HWND window) noexcept {
  base::un<Keyboard> keyboard{std::make_unique<Keyboard>(window)};
  return !keyboard->error_code() ? KeyboardNewResult{std::move(keyboard)}
                                 : KeyboardNewResult{keyboard->error_code()};
}

/**
 * @brief Creates keyboard device.
 * @param window Window to handle keyboard input.
 * @return nothing.
 */
Keyboard::Keyboard(_In_ HWND window) noexcept
    : window_{window},
      error_code_{RegisterRawInputDevices(
          // Disable app hotkeys results in Windows keys passed to the app as we
          // need (or Win keys will invoke standart Windows behavior and
          // interrupt gameplay).
          CreateKeyboardDeviceDefinition(window, RIDEV_NOHOTKEYS))} {
  G3DCHECK(!error_code()) << "Unable to register raw keyboard handler: "
                          << error_code().message();
}

/**
 * @brief Shut down keyboard device.
 */
Keyboard::~Keyboard() noexcept {
  if (!error_code()) {
    // "If a RAWINPUTDEVICE structure has the RIDEV_REMOVE flag set and the
    // hwndTarget parameter is not set to NULL, then parameter validation will
    // fail."
    //
    // See
    // https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerrawinputdevices#remarks
    [[maybe_unused]] const auto rc = RegisterRawInputDevices(
        CreateKeyboardDeviceDefinition(nullptr, RIDEV_REMOVE));
    G3CHECK(!rc) << "Unable to unregister raw keyboard handler: "
                 << rc.message();
  }
}

/**
 * @brief Move constructor.
 * @param k Keyboard.
 * @return nothing.
 */
Keyboard::Keyboard(Keyboard&& k) noexcept
    : window_{k.window_}, error_code_{std::move(k.error_code_)} {
  k.window_ = nullptr;
  k.error_code_ = std::error_code{EINVAL, std::system_category()};
}

/**
 * @brief Handle raw input.
 * @param raw_input Raw input.
 * @param keyboard_input Raw input as keyboard input if it is keyboard input.
 * @return true if raw input is keyboard input, false otherwise.
 */
[[nodiscard]] bool Keyboard::Handle(const RAWINPUT& raw_input,
                                    KeyboardInput& keyboard_input) noexcept {
  if (raw_input.header.dwType == RIM_TYPEKEYBOARD) {
    const auto& keyboard = raw_input.data.keyboard;

    keyboard_input.make_code = keyboard.MakeCode;

    static_assert(std::is_same_v<
                  decltype(keyboard.Flags),
                  std::underlying_type_t<decltype(keyboard_input.key_flags)>>);
    keyboard_input.key_flags = static_cast<KeyboardKeyFlags>(keyboard.Flags);

    // Remap key down event to allow check it via & operator.
    // If key down or not key up + E0 / E1 than it means key down.
    if (keyboard.Flags == RI_KEY_MAKE ||
        !(keyboard.Flags & RI_KEY_BREAK) &&
            ((keyboard.Flags & RI_KEY_E0) || (keyboard.Flags & RI_KEY_E1))) {
      // Fix this one if Windows will use our custom flag.  App will be
      // terminated, fix required (or we will assume key down when it is not).
      G3CHECK(
          !(keyboard.Flags & base::underlying_cast(KeyboardKeyFlags::kDown)))
          << "Windows raw input keyboard data contains flags used for app "
             "itself.  It means app can't detect key down in some cases.  "
             "Please, contact support.";

      keyboard_input.key_flags =
          keyboard_input.key_flags | KeyboardKeyFlags::kDown;
    }

    keyboard_input.virtual_key = keyboard.VKey;
    keyboard_input.message = keyboard.Message;

    return true;
  }

  return false;
}
}  // namespace wb::hal::hid