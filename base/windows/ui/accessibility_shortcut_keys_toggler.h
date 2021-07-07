// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Toggles Windows Accessibility Shortcut Keys.

#ifndef WB_BASE_WINDOWS_UI_ACCESSIBILITY_SHORTCUT_KEYS_TOGGLER_H_
#define WB_BASE_WINDOWS_UI_ACCESSIBILITY_SHORTCUT_KEYS_TOGGLER_H_

#include <cstddef>  // std::byte
#include <optional>
#include <system_error>

#include "base/base_api.h"
#include "base/base_macroses.h"
#include "base/windows/windows_light.h"

namespace wb::base::windows::ui {
/**
 * @brief Toggles Windows Accessibility Shortcut Keys.  See
 * https://docs.microsoft.com/en-us/windows/desktop/dxtecharts/disabling-shortcut-keys-in-games#disable-the-accessibility-shortcut-keys
 */
class AccessibilityShortcutKeysToggler {
 public:
  /**
   * @brief Creates Windows Accessibility Shortcut Keys Toggler.
   * @return nothing.
   */
  WB_BASE_API AccessibilityShortcutKeysToggler() noexcept;
  WB_BASE_API ~AccessibilityShortcutKeysToggler() noexcept;

  AccessibilityShortcutKeysToggler(
      AccessibilityShortcutKeysToggler&&) noexcept = default;
  AccessibilityShortcutKeysToggler& operator=(
      AccessibilityShortcutKeysToggler&&) noexcept = default;

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(AccessibilityShortcutKeysToggler);

  /**
   * @brief Toggles Windows Accessibility Shortcut Keys on/off.
   * @param toggle Toggle on/off.
   * @return Is toggle successfull?
   */
  WB_BASE_API bool Toggle(bool toggle) noexcept;

 private:
  // Small wrapper for better semantics.
  using nullable_bool = std::optional<bool>;

  STICKYKEYS startup_sticky_keys_;
  TOGGLEKEYS startup_toggle_keys_;
  FILTERKEYS startup_filter_keys_;

  std::error_code error_code_;

  nullable_bool is_toggled_;
  [[maybe_unused]] std::byte pad_[6];
};
}  // namespace wb::base::windows::ui

#endif  // !WB_BASE_WINDOWS_UI_ACCESSIBILITY_SHORTCUT_KEYS_TOGGLER_H_
