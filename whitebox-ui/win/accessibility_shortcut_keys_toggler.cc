// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Toggles Windows Accessibility Shortcut Keys.

#include "accessibility_shortcut_keys_toggler.h"

#include <array>
#include <cstddef>  // std::byte
#include <optional>
#include <type_traits>

#include "base/deps/g3log/g3log.h"
#include "base/win/system_error_ext.h"
#include "base/win/windows_light.h"

namespace {

/**
 * @brief Is sticky system key?
 * @tparam TSystemKey key.
 */
template <unsigned key_action, typename TSystemKey>
constexpr bool is_sticky_system_key{
    (key_action == SPI_GETSTICKYKEYS || key_action == SPI_SETSTICKYKEYS) &&
    std::is_same_v<TSystemKey, STICKYKEYS>};

/**
 * @brief Is toggle system key?
 * @tparam TSystemKey key.
 */
template <unsigned key_action, typename TSystemKey>
constexpr bool is_toggle_system_key{
    (key_action == SPI_GETTOGGLEKEYS || key_action == SPI_SETTOGGLEKEYS) &&
    std::is_same_v<TSystemKey, TOGGLEKEYS>};

/**
 * @brief Is filter system key?
 * @tparam TSystemKey key.
 */
template <unsigned key_action, typename TSystemKey>
constexpr bool is_filter_system_key{
    (key_action == SPI_GETFILTERKEYS || key_action == SPI_SETFILTERKEYS) &&
    std::is_same_v<TSystemKey, FILTERKEYS>};

/**
 * @brief Is system key?
 * @tparam TSystemKey key.
 * @tparam R result type if TSystemKey is Windows Accessibility Shortcut key.
 */
template <unsigned key_action, typename TSystemKey, typename R>
using system_key_concept =
    std::enable_if_t<is_sticky_system_key<key_action, TSystemKey> ||
                         is_toggle_system_key<key_action, TSystemKey> ||
                         is_filter_system_key<key_action, TSystemKey>,
                     R>;

template <unsigned key_action, typename TSystemKey>
system_key_concept<key_action, TSystemKey, std::error_code> SystemKeysInfo(
    _In_ TSystemKey& key) noexcept {
  return wb::base::windows::get_error(
      ::SystemParametersInfo(key_action, sizeof(key), &key, 0));
}

}  // namespace

namespace wb::ui::win {

/**
 * @brief Windows accessibility shortcut keys toggler implementation.  See
 * https://docs.microsoft.com/en-us/windows/desktop/dxtecharts/disabling-shortcut-keys-in-games#disable-the-accessibility-shortcut-keys
 */
class AccessibilityShortcutKeysToggler::AccessibilityShortcutKeysTogglerImpl {
 public:
  /**
   * @brief Creates windows accessibility shortcut keys Toggler.
   * @return nothing.
   */
  AccessibilityShortcutKeysTogglerImpl() noexcept;
  ~AccessibilityShortcutKeysTogglerImpl() noexcept;

  AccessibilityShortcutKeysTogglerImpl(
      AccessibilityShortcutKeysTogglerImpl&&) noexcept = default;
  AccessibilityShortcutKeysTogglerImpl& operator=(
      AccessibilityShortcutKeysTogglerImpl&&) noexcept = default;

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(AccessibilityShortcutKeysTogglerImpl);

  /**
   * @brief Toggles windows accessibility shortcut keys on/off.
   * @param toggle Toggle on/off.
   * @return Is toggle successfull?
   */
  bool Toggle(bool toggle) noexcept;

 private:
  // Small wrapper for better semantics.
  using nullable_bool = std::optional<bool>;

  STICKYKEYS startup_sticky_keys_;
  TOGGLEKEYS startup_toggle_keys_;
  FILTERKEYS startup_filter_keys_;

  std::error_code error_code_;

  nullable_bool is_toggled_;

  [[maybe_unused]] std::array<std::byte, sizeof(char*) - sizeof(nullable_bool)>
      pad_;
};

AccessibilityShortcutKeysToggler::AccessibilityShortcutKeysTogglerImpl::
    AccessibilityShortcutKeysTogglerImpl() noexcept
    : startup_sticky_keys_{sizeof(decltype(startup_sticky_keys_)), 0},
      startup_toggle_keys_{sizeof(decltype(startup_toggle_keys_)), 0},
      startup_filter_keys_{sizeof(decltype(startup_filter_keys_)), 0},
      error_code_{},
      is_toggled_{} {
  // Save the current sticky/toggle/filter key settings so they can be restored
  // later.
  std::error_code rc{SystemKeysInfo<SPI_GETSTICKYKEYS>(startup_sticky_keys_)};
  G3DCHECK(!rc);

  if (!rc) {
    rc = SystemKeysInfo<SPI_GETTOGGLEKEYS>(startup_toggle_keys_);
    G3DCHECK(!rc);
  }

  if (!rc) {
    rc = SystemKeysInfo<SPI_GETFILTERKEYS>(startup_filter_keys_);
    G3DCHECK(!rc);
  }

  error_code_ = rc;
}

AccessibilityShortcutKeysToggler::AccessibilityShortcutKeysTogglerImpl::
    ~AccessibilityShortcutKeysTogglerImpl() noexcept {
  Toggle(true);
}

bool AccessibilityShortcutKeysToggler::AccessibilityShortcutKeysTogglerImpl::
    Toggle(bool toggle) noexcept {
  if (is_toggled_.has_value() && is_toggled_.value() == toggle) return false;
  if (error_code_) return false;

  if (toggle) {
    // Restore StickyKeys/etc to original state and enable Windows key.
    [[maybe_unused]] auto error_code3 =
        SystemKeysInfo<SPI_SETFILTERKEYS>(startup_filter_keys_);
    G3DCHECK(!error_code3);
    [[maybe_unused]] auto error_code2 =
        SystemKeysInfo<SPI_SETTOGGLEKEYS>(startup_toggle_keys_);
    G3DCHECK(!error_code2);
    [[maybe_unused]] auto error_code1 =
        SystemKeysInfo<SPI_SETSTICKYKEYS>(startup_sticky_keys_);
    G3DCHECK(!error_code1);
  } else {
    // Disable StickyKeys/etc shortcuts but if the accessibility feature is
    // on, then leave the settings alone as its probably being usefully used.

    STICKYKEYS sticky_keys_off = startup_sticky_keys_;
    if ((sticky_keys_off.dwFlags & SKF_STICKYKEYSON) == 0) {
      // Disable the hotkey and the confirmation.
      sticky_keys_off.dwFlags &= ~SKF_HOTKEYACTIVE;
      sticky_keys_off.dwFlags &= ~SKF_CONFIRMHOTKEY;

      [[maybe_unused]] const auto error_code =
          SystemKeysInfo<SPI_SETSTICKYKEYS>(sticky_keys_off);
      G3DCHECK(!error_code);
    }

    TOGGLEKEYS toggle_keys_off = startup_toggle_keys_;
    if ((toggle_keys_off.dwFlags & TKF_TOGGLEKEYSON) == 0) {
      // Disable the hotkey and the confirmation.
      toggle_keys_off.dwFlags &= ~TKF_HOTKEYACTIVE;
      toggle_keys_off.dwFlags &= ~TKF_CONFIRMHOTKEY;

      [[maybe_unused]] const auto error_code =
          SystemKeysInfo<SPI_SETTOGGLEKEYS>(toggle_keys_off);
      G3DCHECK(!error_code);
    }

    FILTERKEYS filter_keys_off = startup_filter_keys_;
    if ((filter_keys_off.dwFlags & FKF_FILTERKEYSON) == 0) {
      // Disable the hotkey and the confirmation.
      filter_keys_off.dwFlags &= ~FKF_HOTKEYACTIVE;
      filter_keys_off.dwFlags &= ~FKF_CONFIRMHOTKEY;

      [[maybe_unused]] const auto error_code =
          SystemKeysInfo<SPI_SETFILTERKEYS>(filter_keys_off);
      G3DCHECK(!error_code);
    }
  }

  is_toggled_ = toggle;
  return true;
}

AccessibilityShortcutKeysToggler::AccessibilityShortcutKeysToggler() noexcept
    : impl_{std::make_unique<AccessibilityShortcutKeysTogglerImpl>()} {}

AccessibilityShortcutKeysToggler::~AccessibilityShortcutKeysToggler() noexcept =
    default;

AccessibilityShortcutKeysToggler::AccessibilityShortcutKeysToggler(
    AccessibilityShortcutKeysToggler&&) noexcept = default;

AccessibilityShortcutKeysToggler& AccessibilityShortcutKeysToggler::operator=(
    AccessibilityShortcutKeysToggler&&) noexcept = default;

bool AccessibilityShortcutKeysToggler::Toggle(bool toggle) noexcept {
  return impl_->Toggle(toggle);
}

}  // namespace wb::ui::win
