// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Toggles window full/narrow screen.

#ifndef WB_UI_WIN_FULL_SCREEN_WINDOW_TOGGLER_H_
#define WB_UI_WIN_FULL_SCREEN_WINDOW_TOGGLER_H_

#include "base/macroses.h"
#include "build/compiler_config.h"
#include "ui/config.h"

using HWND = struct HWND__ *;

namespace wb::ui::win {

/**
 * @brief Toggles window full/narrow screen.
 */
class WB_WHITEBOX_UI_API FullScreenWindowToggler {
 public:
  FullScreenWindowToggler(_In_ HWND window,
                          _In_ long default_window_style) noexcept;
  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(FullScreenWindowToggler);

  ~FullScreenWindowToggler() noexcept;

  /**
   * @brief Toggle window full/narrow screen.
   * @param toggle Go full screen?
   * @return Old full screen state, true if was full screen, false otherwise.
   */
  bool Toggle(bool toggle) noexcept;

  /**
   * @brief Is window full screen now?
   * @return true if full screen, false otherwise.
   */
  [[nodiscard]] bool IsFullScreen() const noexcept;

 private:
  class FullScreenWindowTogglerImpl;

  WB_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()
    // Private member is not accessible to the DLL's client, including inline
    // functions.
    WB_MSVC_DISABLE_WARNING(4251)
    /**
     * @brief Actual implementation.
     */
    base::un<FullScreenWindowTogglerImpl> impl_;
  WB_MSVC_END_WARNING_OVERRIDE_SCOPE()
};

}  // namespace wb::ui::win

#endif  // !WB_UI_WIN_FULL_SCREEN_WINDOW_TOGGLER_H_