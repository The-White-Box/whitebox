// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Toggles windows accessibility shortcut keys.

#ifndef WB_WHITEBOX_UI_WIN_ACCESSIBILITY_SHORTCUT_KEYS_TOGGLER_H_
#define WB_WHITEBOX_UI_WIN_ACCESSIBILITY_SHORTCUT_KEYS_TOGGLER_H_

#include "base/macroses.h"
#include "build/compiler_config.h"
#include "whitebox-ui/config.h"

namespace wb::ui::win {

/**
 * @brief Toggles windows accessibility shortcut keys.
 */
class WB_WHITEBOX_UI_API AccessibilityShortcutKeysToggler {
 public:
  /**
   * @brief Creates windows accessibility shortcut keys toggler.
   * @return nothing.
   */
  AccessibilityShortcutKeysToggler() noexcept;

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(AccessibilityShortcutKeysToggler);
  AccessibilityShortcutKeysToggler(AccessibilityShortcutKeysToggler&&) noexcept;
  AccessibilityShortcutKeysToggler& operator=(
      AccessibilityShortcutKeysToggler&&) noexcept;

  ~AccessibilityShortcutKeysToggler() noexcept;

  /**
   * @brief Toggles windows accessibility shortcut keys on/off.
   * @param toggle Toggle on/off.
   * @return Is toggle successfull?
   */
  bool Toggle(bool toggle) noexcept;

 private:
  class AccessibilityShortcutKeysTogglerImpl;

  WB_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()
    // Private member is not accessible to the DLL's client, including inline
    // functions.
    WB_MSVC_DISABLE_WARNING(4251)
    /**
     * @brief Actual implementation.
     */
    base::un<AccessibilityShortcutKeysTogglerImpl> impl_;
  WB_MSVC_END_WARNING_OVERRIDE_SCOPE()
};

}  // namespace wb::ui::win

#endif  // !WB_WHITEBOX_UI_WIN_ACCESSIBILITY_SHORTCUT_KEYS_TOGGLER_H_
