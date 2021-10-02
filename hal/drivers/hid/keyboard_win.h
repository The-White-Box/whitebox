// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Keyboard input device.

#ifndef WB_HAL_DRIVERS_HID_KEYBOARD_WIN_H_
#define WB_HAL_DRIVERS_HID_KEYBOARD_WIN_H_

#include <system_error>

#include "base/base_macroses.h"
#include "base/std2/system_error_ext.h"
#include "build/compiler_config.h"
#include "hid_api.h"
#include "keyboard_input.h"

using HWND = struct HWND__ *;
using RAWINPUT = struct tagRAWINPUT;

namespace wb::hal::hid {

/**
 * @brief Low level keyboard input device.
 */
class WB_HAL_HID_DRIVER_API Keyboard {
  // Need to call private ctor.  Safe as ctor is private, so only we can create.
  friend std::unique_ptr<wb::hal::hid::Keyboard>
  std::make_unique<wb::hal::hid::Keyboard, HWND &>(HWND &);

 public:
  /**
   * @brief Creates keyboard device.
   * @param window Window to handle keyboard input.
   * @return Keyboard.
   */
  [[nodiscard]] static base::std2::result<base::un<Keyboard>> New(
      _In_ HWND window) noexcept;

  Keyboard(Keyboard &&) noexcept;
  Keyboard &operator=(Keyboard &&) noexcept = delete;
  WB_NO_COPY_CTOR_AND_ASSIGNMENT(Keyboard);

  /**
   * @brief Shut down keyboard device.
   */
  ~Keyboard() noexcept;

  /**
   * @brief Handle raw input.
   * @param raw_input Raw input.
   * @param keyboard_input Raw input as keyboard input if it is keyboard input.
   * @return true if raw input is keyboard input, false otherwise.
   */
  [[nodiscard]] bool Handle(const RAWINPUT &raw_input,
                            KeyboardInput &keyboard_input) noexcept;

 private:
  /**
   * @brief Window handle to get keyboard input for.
   */
  HWND window_;

  WB_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()
    // Private member is not accessible to the DLL's client, including inline
    // functions.
    WB_MSVC_DISABLE_WARNING(4251)
    /**
     * @brief Keyboard initialization error code.
     */
    std::error_code error_code_;
  WB_MSVC_END_WARNING_OVERRIDE_SCOPE()

  /**
   * @brief Creates keyboard device.
   * @param window Window to handle keyboard input.
   * @return nothing.
   */
  Keyboard(_In_ HWND window) noexcept;

  /**
   * @brief Keyboard initialization error code.
   * @return Error code.
   */
  [[nodiscard]] std::error_code error_code() const noexcept {
    return error_code_;
  }
};

}  // namespace wb::hal::hid

#endif  // !WB_HAL_DRIVERS_HID_KEYBOARD_WIN_H_
