// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// <system_error> extensions for Windows.

#ifndef WB_BASE_INCLUDE_WINDOWS_SYSTEM_ERROR_EXT_H_
#define WB_BASE_INCLUDE_WINDOWS_SYSTEM_ERROR_EXT_H_

#include "base/include/system_error_ext.h"

/**
 * @brief HWND type.
 */
using HWND = struct HWND__*;

/**
 * @brief BOOL type.
 */
using BOOL = int;

namespace wb::base::windows {
/**
 * @brief Get error code.
 */
template <typename R>
[[nodiscard]] inline std::error_code GetErrorCode(_In_ R result) noexcept =
    delete;

/**
 * @brief Get error code.
 */
template <>
[[nodiscard]] inline std::error_code GetErrorCode(
    _In_ unsigned short result) noexcept {
  return result ? std::error_code{} : GetSystemErrorCode();
}

/**
 * @brief Get error code.
 */
template <>
[[nodiscard]] inline std::error_code GetErrorCode(
    _In_opt_ HWND result) noexcept {
  return result ? std::error_code{} : GetSystemErrorCode();
}

/**
 * @brief Get error code.
 */
template <>
[[nodiscard]] inline std::error_code GetErrorCode(_In_ BOOL result) noexcept {
  return result ? std::error_code{} : GetSystemErrorCode();
}
}  // namespace wb::base::windows

#endif  // !WB_BASE_INCLUDE_WINDOWS_SYSTEM_ERROR_EXT_H_
