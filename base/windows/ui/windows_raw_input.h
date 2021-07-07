// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Windows Raw Input API wrappers.

#ifndef WB_BASE_WINDOWS_UI_WINDOWS_RAW_INPUT_H_
#define WB_BASE_WINDOWS_UI_WINDOWS_RAW_INPUT_H_

#include <cstdint>
#include <system_error>

#include "base/base_api.h"

using RAWINPUTDEVICE = struct tagRAWINPUTDEVICE;
using HRAWINPUT = struct HRAWINPUT__*;
using RAWINPUT = struct tagRAWINPUT;

using LRESULT = std::intptr_t;

namespace wb::base::windows::ui {
/**
 * @brief Register raw input device.
 * @param device Device.
 * @return Error code.
 */
[[nodiscard]] WB_BASE_API std::error_code RegisterRawInputDevices(
    _In_ const RAWINPUTDEVICE& device) noexcept;

/**
 * @brief Handles unhandled raw input.
 * @param header_size Header size.
 * @return 0 on success, -1 on failure.
 */
[[nodiscard]] WB_BASE_API LRESULT
HandleNonHandledRawInput(unsigned header_size) noexcept;

/**
 * @brief Read raw input.
 * @param source_input Input source.
 * @param read_input Input.
 * @return true on success, false on failure.
 */
[[nodiscard]] WB_BASE_API bool ReadRawInput(_In_ HRAWINPUT source_input,
                                            RAWINPUT& read_input) noexcept;
}  // namespace wb::base::windows::ui

#endif  // !WB_BASE_WINDOWS_UI_WINDOWS_RAW_INPUT_H_
