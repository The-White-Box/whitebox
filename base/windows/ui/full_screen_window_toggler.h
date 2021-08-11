// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Toggles window full/narrow screen.

#ifndef WB_BASE_WINDOWS_UI_FULL_SCREEN_WINDOW_TOGGLER_H_
#define WB_BASE_WINDOWS_UI_FULL_SCREEN_WINDOW_TOGGLER_H_

#include "base/base_api.h"
#include "base/base_macroses.h"
#include "build/compiler_config.h"

using HWND = struct HWND__ *;

namespace wb::base::windows::ui {
/**
 * @brief Toggles window full/narrow screen.
 */
class WB_BASE_API FullScreenWindowToggler {
 public:
  FullScreenWindowToggler(_In_ HWND window,
                          _In_ long default_window_style) noexcept;

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(FullScreenWindowToggler);

  ~FullScreenWindowToggler() noexcept;

  /**
   * @brief Toggle window full/narrow screen.
   * @param toggle Go full screen?
   * @return void.
   */
  void Toggle(bool toggle) noexcept;

  /**
   * @brief Is window full screen now?
   * @return true if full screen, false otherwise.
   */
  [[nodiscard]] bool IsFullScreen() const noexcept;

 private:
  class FullScreenWindowTogglerImpl;

  WB_COMPILER_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()
    // Private member is not accessible to the DLL's client, including inline
    // functions.
    WB_COMPILER_MSVC_DISABLE_WARNING(4251)
    /**
     * @brief Actual implementation.
     */
    wb::base::un<FullScreenWindowTogglerImpl> impl_;
  WB_COMPILER_MSVC_END_WARNING_OVERRIDE_SCOPE()
};
}  // namespace wb::base::windows::ui

#endif  // !WB_BASE_WINDOWS_UI_FULL_SCREEN_WINDOW_TOGGLER_H_