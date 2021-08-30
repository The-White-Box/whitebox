// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Mouse input device.

#ifndef WB_HAL_DRIVERS_HID_MOUSE_WIN_H_
#define WB_HAL_DRIVERS_HID_MOUSE_WIN_H_

#include <system_error>

#include "base/base_macroses.h"
#include "base/std_ext/system_error_ext.h"
#include "hid_api.h"
#include "mouse_input.h"

using HWND = struct HWND__ *;
using RAWINPUT = struct tagRAWINPUT;

namespace wb::hal::hid {
/**
 * @brief Low level mouse input device.
 */
class WB_HAL_HID_DRIVER_API Mouse {
  // Need to call private ctor.  Safe as ctor is private, so only we can create.
  friend std::unique_ptr<wb::hal::hid::Mouse>
  std::make_unique<wb::hal::hid::Mouse, HWND &>(HWND &);

 public:
  /**
   * @brief Creates mouse device.
   * @param window Window to handle mouse input.
   * @return Mouse.
   */
  [[nodiscard]] static base::std_ext::os_res<base::un<Mouse>> New(
      _In_ HWND window) noexcept;

  Mouse(Mouse &&) noexcept;
  Mouse &operator=(Mouse &&) noexcept = default;
  WB_NO_COPY_CTOR_AND_ASSIGNMENT(Mouse);

  /**
   * @brief Shut down mouse device.
   */
  ~Mouse() noexcept;

  /**
   * @brief Handle raw input.
   * @param raw_input Raw input.
   * @param mouse_input Raw input as mouse input if it is mouse input.
   * @return true if raw input is mouse input, false otherwise.
   */
  [[nodiscard]] bool Handle(const RAWINPUT &raw_input,
                            MouseInput &mouse_input) noexcept;

 private:
  /**
   * @brief Window handle to get mouse input for.
   */
  HWND window_;

  WB_COMPILER_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()
    // Private member is not accessible to the DLL's client, including inline
    // functions.
    WB_COMPILER_MSVC_DISABLE_WARNING(4251)
    /**
     * @brief Mouse initialization error code.
     */
    std::error_code error_code_;
  WB_COMPILER_MSVC_END_WARNING_OVERRIDE_SCOPE()

  /**
   * @brief Last absolute mouse x, y.  May be invalid if no absolute mouse
   * movement present.
   */
  long last_absolute_x_, last_absolute_y_;

  /**
   * @brief Creates mouse device.
   * @param window Window to handle mouse input.
   * @return nothing.
   */
  Mouse(_In_ HWND window) noexcept;

  /**
   * @brief Mouse initialization error code.
   * @return Error code.
   */
  [[nodiscard]] std::error_code error_code() const noexcept {
    return error_code_;
  }
};
}  // namespace wb::hal::hid

#endif  // !WB_HAL_DRIVERS_HID_MOUSE_WIN_H_