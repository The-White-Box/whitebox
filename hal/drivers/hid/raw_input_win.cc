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

[[nodiscard]] WB_HAL_HID_DRIVER_API std::error_code RegisterRawInputDevices(
    _In_ const RAWINPUTDEVICE& device) noexcept {
  return base::win::get_error(
      ::RegisterRawInputDevices(&device, 1U, sizeof(device)));
}

[[nodiscard]] WB_HAL_HID_DRIVER_API LRESULT
HandleNonHandledRawInput(_In_ unsigned header_size) noexcept {
  // First two parameters are ignored.
  return ::DefRawInputProc(nullptr, 0, header_size);
}

[[nodiscard]] WB_HAL_HID_DRIVER_API std::uint32_t ReadRawInput(
    _In_ HRAWINPUT source_input, RAWINPUT& read_input) noexcept {
  G3DCHECK(!!source_input);

  unsigned copied_bytes_count{sizeof(read_input)};
  const std::uint32_t copied_bytes{
      ::GetRawInputData(source_input, RID_INPUT, &read_input,
                        &copied_bytes_count, sizeof(read_input.header))};

  G3DCHECK(copied_bytes != 0);
  G3DCHECK(copied_bytes != static_cast<decltype(copied_bytes)>(-1));

  return copied_bytes;
}

}  // namespace wb::hal::hid
