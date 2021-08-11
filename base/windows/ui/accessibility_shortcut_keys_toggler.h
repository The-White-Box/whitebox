// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Toggles windows accessibility shortcut keys.

#ifndef WB_BASE_WINDOWS_UI_ACCESSIBILITY_SHORTCUT_KEYS_TOGGLER_H_
#define WB_BASE_WINDOWS_UI_ACCESSIBILITY_SHORTCUT_KEYS_TOGGLER_H_

#include "base/base_api.h"
#include "base/base_macroses.h"
#include "build/compiler_config.h"

namespace wb::base::windows::ui {
/**
 * @brief Toggles windows accessibility shortcut keys.
 */
class WB_BASE_API AccessibilityShortcutKeysToggler {
 public:
  /**
   * @brief Creates windows accessibility shortcut keys toggler.
   * @return nothing.
   */
  AccessibilityShortcutKeysToggler() noexcept;
  AccessibilityShortcutKeysToggler(AccessibilityShortcutKeysToggler&&) noexcept;
  AccessibilityShortcutKeysToggler& operator=(
      AccessibilityShortcutKeysToggler&&) noexcept;

  ~AccessibilityShortcutKeysToggler() noexcept;

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(AccessibilityShortcutKeysToggler);

  /**
   * @brief Toggles windows accessibility shortcut keys on/off.
   * @param toggle Toggle on/off.
   * @return Is toggle successfull?
   */
  bool Toggle(bool toggle) noexcept;

 private:
  class AccessibilityShortcutKeysTogglerImpl;

  WB_COMPILER_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()
    // Private member is not accessible to the DLL's client, including inline
    // functions.
    WB_COMPILER_MSVC_DISABLE_WARNING(4251)
    /**
     * @brief Actual implementation.
     */
    wb::base::un<AccessibilityShortcutKeysTogglerImpl> impl_;
  WB_COMPILER_MSVC_END_WARNING_OVERRIDE_SCOPE()
};
}  // namespace wb::base::windows::ui

#endif  // !WB_BASE_WINDOWS_UI_ACCESSIBILITY_SHORTCUT_KEYS_TOGGLER_H_
