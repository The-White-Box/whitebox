// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Windows Raw Input API wrappers.

#ifndef WB_HAL_DRIVERS_HID_RAW_INPUT_WIN_H_
#define WB_HAL_DRIVERS_HID_RAW_INPUT_WIN_H_

#include <cstdint>
#include <system_error>

#include "hid_api.h"

using RAWINPUT = struct tagRAWINPUT;
using RAWINPUTDEVICE = struct tagRAWINPUTDEVICE;
using HRAWINPUT = struct HRAWINPUT__*;

using LRESULT = std::intptr_t;

namespace wb::hal::hid {

/**
 * @brief Register raw input device.
 * @param device Device.
 * @return Error code.
 */
[[nodiscard]] WB_HAL_HID_DRIVER_API std::error_code RegisterRawInputDevices(
    _In_ const RAWINPUTDEVICE& device) noexcept;

/**
 * @brief Handles unhandled raw input.
 * @param header_size Header size.
 * @return 0 on success, -1 on failure.
 */
[[nodiscard]] WB_HAL_HID_DRIVER_API LRESULT
HandleNonHandledRawInput(_In_ unsigned header_size) noexcept;

/**
 * @brief Read raw input.
 * @param source_input Input source.
 * @param read_input Input.
 * @return Copied into |read_input| bytes.  0 on error.
 */
[[nodiscard]] WB_HAL_HID_DRIVER_API std::uint32_t ReadRawInput(
    _In_ HRAWINPUT source_input, RAWINPUT& read_input) noexcept;

}  // namespace wb::hal::hid

#endif  // !WB_HAL_DRIVERS_HID_RAW_INPUT_WIN_H_
