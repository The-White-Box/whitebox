// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// <system_error> extensions for Windows.

#ifndef WB_BASE_WIN_SYSTEM_ERROR_EXT_H_
#define WB_BASE_WIN_SYSTEM_ERROR_EXT_H_

#include "build/compiler_config.h"

WB_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()
  // 'AAA': conversion from 'BBB' to 'CCC', signed/unsigned mismatch
  WB_MSVC_DISABLE_WARNING(4365)
  // 'XXX': '4' bytes padding added after data member 'ZZZ'
  WB_MSVC_DISABLE_WARNING(4820)
  // 'SSS': class has virtual functions, but its trivial destructor
  // is not virtual
  WB_MSVC_DISABLE_WARNING(5204)
#include <comdef.h>
WB_MSVC_END_WARNING_OVERRIDE_SCOPE()

#include <string>
#include <system_error>

#include "base/macroses.h"
#include "base/std2/system_error_ext.h"

/**
 * @brief HWND type.
 */
using HWND = struct HWND__ *;

/**
 * @brief BOOL type.
 */
using BOOL = int;

/**
 * @brief HHOOK type.
 */
using HHOOK = struct HHOOK__ *;

struct HINSTANCE__;

/**
 * @brief HINSTANCE type.
 */
using HINSTANCE = struct HINSTANCE__ *;

/**
 * @brief HMODULE type.
 */
using HMODULE = HINSTANCE;

/**
 * @brief HRESULT type.
 */
using HRESULT = long;

namespace wb::base::win {

/**
 * @brief Generic test for success on any status value (non-negative numbers
 * indicate success).
 * @param hr HRESULT.
 * @return true if succeeded, false otherwise.
 */
[[nodiscard]] constexpr bool is_succeeded(HRESULT hr) noexcept {
  return hr >= 0;
}

/**
 * @brief Generic test for failure on any status value (negative numbers
 * indicate failure).
 * @param hr HRESULT.
 * @return true if failed, false otherwise.
 */
[[nodiscard]] constexpr bool is_failed(HRESULT hr) noexcept { return hr < 0; }

/**
 * @brief Get error code.
 */
template <typename R>
[[nodiscard]] inline std::error_code get_error(_In_ R result) noexcept = delete;

/**
 * @brief Get error code.
 */
template <>
[[nodiscard]] inline std::error_code get_error(
    _In_ unsigned short result) noexcept {
  return result ? std2::ok_code : std2::system_last_error_code();
}

/**
 * @brief Get error code.
 */
template <>
[[nodiscard]] inline std::error_code get_error(_In_opt_ HWND result) noexcept {
  return result ? std2::ok_code : std2::system_last_error_code();
}

/**
 * @brief Get error code.
 */
template <>
[[nodiscard]] inline std::error_code get_error(_In_ BOOL result) noexcept {
  return result ? std2::ok_code : std2::system_last_error_code();
}

/**
 * @brief Get error code.
 */
template <>
[[nodiscard]] inline std::error_code get_error(_In_opt_ HHOOK result) noexcept {
  return result ? std2::ok_code : std2::system_last_error_code();
}

/**
 * @brief Get error code.
 */
template <>
[[nodiscard]] inline std::error_code get_error(
    _In_opt_ HINSTANCE result) noexcept {
  return result ? std2::ok_code : std2::system_last_error_code();
}

/**
 * @brief Component Object Model error category.
 */
class ComErrorCategory : public std::error_category {
 public:
  ComErrorCategory() noexcept = default;

  /**
   * @brief Error category name.
   * @return Category name.
   */
  [[nodiscard]] const char *name() const noexcept override { return "com"; }

  /**
   * @brief Gets error value HRESULT description.
   * @param error_value HRESULT.
   * @return Error description.
   */
  [[nodiscard]] std::string message(int error_value) const override {
    const _com_error com_error{static_cast<HRESULT>(error_value)};
    return std::string{com_error.ErrorMessage()};
  }

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ComErrorCategory);
};

/**
 * @brief Gets COM error category singleton.
 * @return COM error category singleton.
 */
[[nodiscard]] inline ComErrorCategory &com_error_category() {
  static ComErrorCategory com_error_category;
  return com_error_category;
}

/**
 * @brief Get COM error code.
 * @param rc Native system COM error code.
 * @return System COM error code.
 */
[[nodiscard]] inline std::error_code get_com_error_code(
    const HRESULT result) noexcept {
  return std::error_code{result, com_error_category()};
}

/**
 * @brief Get COM error code by HRESULT.
 */
template <>
[[nodiscard]] inline std::error_code get_error(_In_ HRESULT result) noexcept {
  return is_succeeded(result) ? std2::ok_code : get_com_error_code(result);
}

}  // namespace wb::base::win

#endif  // !WB_BASE_WIN_SYSTEM_ERROR_EXT_H_
