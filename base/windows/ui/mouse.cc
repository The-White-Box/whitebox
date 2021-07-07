// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Mouse input device.

#include "mouse.h"

#include "base/deps/g3log/g3log.h"
#include "base/windows/ui/windows_raw_input.h"
#include "base/windows/windows_light.h"
// Should go after windows.h
#include <hidusage.h>

namespace {
/**
 * @brief Create mouse device definition.
 * @param window Window.
 * @param flags Mouse device flags.
 * @return Mouse device definition.
 */
[[nodiscard]] RAWINPUTDEVICE CreateMouseDeviceDefinition(
    _In_opt_ HWND window, _In_ unsigned long flags) {
  return RAWINPUTDEVICE{.usUsagePage = HID_USAGE_PAGE_GENERIC,
                        .usUsage = HID_USAGE_GENERIC_MOUSE,
                        .dwFlags = flags,
                        .hwndTarget = window};
}
}  // namespace

namespace wb::base::windows::ui {
/**
 * @brief Creates mouse device.
 * @param window Window to handle mouse input.
 * @return nothing.
 */
WB_BASE_API Mouse::Mouse(_In_ HWND window) noexcept
    : window_{window},
      error_code_{
          RegisterRawInputDevices(CreateMouseDeviceDefinition(window, 0))} {
  G3DCHECK(!error_code()) << "Unable to register raw mouse handler: "
                          << error_code().message();
}

/**
 * @brief Shut down mouse device.
 */
WB_BASE_API Mouse::~Mouse() noexcept {
  if (!error_code()) {
    // "If a RAWINPUTDEVICE structure has the RIDEV_REMOVE flag set and the
    // hwndTarget parameter is not set to NULL, then parameter validation will
    // fail."
    //
    // See
    // https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerrawinputdevices#remarks
    [[maybe_unused]] const auto rc = RegisterRawInputDevices(
        CreateMouseDeviceDefinition(nullptr, RIDEV_REMOVE));
    G3CHECK(!rc) << "Unable to unregister raw mouse handler: " << rc.message();
  }
}

/**
 * @brief Handle raw input.
 * @param raw_input Raw input.
 * @param mouse_input Raw input as mouse input if it is mouse input.
 * @return true if raw input is mouse input, false otherwise.
 */
[[nodiscard]] WB_BASE_API bool Mouse::Handle(const RAWINPUT& raw_input,
                                             MouseInput& mouse_input) noexcept {
  if (raw_input.header.dwType == RIM_TYPEMOUSE) {
    const auto& mouse = raw_input.data.mouse;

    static_assert(
        std::is_same_v<decltype(mouse.usFlags), std::underlying_type_t<decltype(
                                                    mouse_input.mouse_state)>>);
    mouse_input.mouse_state = static_cast<MouseStateFlags>(mouse.usFlags);

    // Remap relative mouse move flag to allow check it via & operator.
    // If relative move or not absolute and has move data.
    if (mouse.usFlags == MOUSE_MOVE_RELATIVE ||
        (!(mouse.usFlags & MOUSE_MOVE_ABSOLUTE) &&
         (mouse.lLastX != 0 || mouse.lLastY != 0))) {
      // Fix this one if Windows will use our custom flag.  App will be
      // terminated, fix required (or we will assume relative mouse move when it
      // is not).
      G3CHECK(
          !(mouse.usFlags & underlying_cast(MouseStateFlags::kMoveRelative)))
          << "Windows raw input mouse data contains flags used for app itself. "
             " It means app can't detect relative mouse movement in some "
             "cases.  Please, contact support.";

      mouse_input.mouse_state =
          mouse_input.mouse_state | MouseStateFlags::kMoveRelative;
    }

    static_assert(std::is_same_v<
                  decltype(mouse.usButtonFlags),
                  std::underlying_type_t<decltype(mouse_input.button_flags)>>);
    const auto button_flags =
        static_cast<MouseButtonTransitionState>(mouse.usButtonFlags);
    mouse_input.button_flags = button_flags;

    if ((button_flags & MouseButtonTransitionState::kVerticalWheel) ==
            MouseButtonTransitionState::kVerticalWheel ||
        (button_flags & MouseButtonTransitionState::kHorizontalWheel) ==
            MouseButtonTransitionState::kHorizontalWheel) {
      // High precision touchpads / mouse wheels can have usButtonData in units
      // less than WHEEL_DELTA.  The delta was set to 120 to allow Microsoft or
      // other vendors to build finer-resolution wheels (a freely-rotating wheel
      // with no notches) to send more messages per rotation, but with a smaller
      // value in each message.  To use this feature, you can either add the
      // incoming delta values until WHEEL_DELTA is reached (so for a
      // delta-rotation you get the same response), or scroll partial lines in
      // response to the more frequent messages.  You can also choose your
      // scroll granularity and accumulate deltas until it is reached.
      mouse_input.button_data =
          static_cast<float>(static_cast<short>(mouse.usButtonData)) /
          WHEEL_DELTA;
    } else {
      mouse_input.button_data = 0.0F;
    }

    mouse_input.last_x = mouse.lLastX;
    mouse_input.last_y = mouse.lLastY;

    return true;
  }

  return false;
}
}  // namespace wb::base::windows::ui