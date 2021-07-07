// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Toggles window full/narrow screen.

#ifndef WB_BASE_WINDOWS_UI_FULL_SCREEN_WINDOW_TOGGLER_H_
#define WB_BASE_WINDOWS_UI_FULL_SCREEN_WINDOW_TOGGLER_H_

#include <cstddef>  // std::byte

#include "base/base_api.h"
#include "base/base_macroses.h"
#include "base/windows/windows_light.h"

namespace wb::base::windows::ui {
/**
 * @brief Toggles window full/narrow screen.
 */
class WB_BASE_API FullScreenWindowToggler {
 public:
  FullScreenWindowToggler(_In_ HWND window,
                          _In_ LONG default_window_style) noexcept;

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(FullScreenWindowToggler);

  ~FullScreenWindowToggler() noexcept = default;

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
  [[nodiscard]] bool IsFullScreen() const noexcept {
    return is_fullscreen_now_;
  }

 private:
  const HWND window_;
  const LONG default_window_style_;

  WINDOWPLACEMENT narrow_window_placement_;
  bool is_fullscreen_now_;

  std::byte pad[sizeof(char*) - sizeof(is_fullscreen_now_)];

  [[nodiscard]] bool SetWindowStyle(_In_ LONG_PTR window_style) const noexcept;

  void GoFullScreen(_In_ LONG_PTR window_style) noexcept;

  void GoNarrowScreen(_In_ LONG_PTR window_style) noexcept;
};
}  // namespace wb::base::windows::ui

#endif  // !WB_BASE_WINDOWS_UI_FULL_SCREEN_WINDOW_TOGGLER_H_