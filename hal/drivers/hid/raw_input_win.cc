// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Windows Raw Input API wrappers.

#include "raw_input_win.h"

#include "base/deps/g3log/g3log.h"
#include "base/win/system_error_ext.h"
#include "base/win/windows_light.h"

namespace wb::hal::hid {

/**
 * @brief Register raw input device.
 * @param device Device.
 * @return Error code.
 */
[[nodiscard]] WB_HAL_HID_DRIVER_API std::error_code RegisterRawInputDevices(
    _In_ const RAWINPUTDEVICE& device) noexcept {
  return base::win::get_error(
      ::RegisterRawInputDevices(&device, 1, sizeof(device)));
}

/**
 * @brief Handles unhandled raw input.
 * @param header_size Header size.
 * @return 0 on success, -1 on failure.
 */
[[nodiscard]] WB_HAL_HID_DRIVER_API LRESULT
HandleNonHandledRawInput(unsigned header_size) noexcept {
  // First two parameters are ignored.
  return ::DefRawInputProc(nullptr, 0, header_size);
}

/**
 * @brief Read raw input.
 * @param source_input Input source.
 * @param read_input Input.
 * @return true on success, false on failure.
 */
[[nodiscard]] WB_HAL_HID_DRIVER_API bool ReadRawInput(
    _In_ HRAWINPUT source_input, RAWINPUT& read_input) noexcept {
  G3DCHECK(!!source_input);

  constexpr unsigned kRawInputError{static_cast<unsigned>(-1)};

  unsigned read_size{sizeof(read_input)};
  const unsigned error_code{::GetRawInputData(source_input, RID_INPUT,
                                              &read_input, &read_size,
                                              sizeof(read_input.header))};

  G3DCHECK(error_code != kRawInputError);

  return error_code != kRawInputError;
}

}  // namespace wb::hal::hid
