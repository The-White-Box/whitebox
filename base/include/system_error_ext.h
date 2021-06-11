// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// <system_error> extensions.

#ifndef WB_BASE_INCLUDE_SYSTEM_ERROR_EXT_H_
#define WB_BASE_INCLUDE_SYSTEM_ERROR_EXT_H_

#include <system_error>
#include <variant>

#include "build/include/build_config.h"

#ifdef WB_OS_WIN
#include <sal.h>          // _Check_return_
#include <specstrings.h>  // _Post_equals_last_error_

extern "C" __declspec(dllimport) _Check_return_ _Post_equals_last_error_
    unsigned long __stdcall GetLastError(void);
#else
#include <cerrno>
#endif

namespace wb::base {
/**
 * @brief Get last native system errno.
 * @return Last native system errno.
 */
[[nodiscard]] inline int GetLastNativeErrno() noexcept {
#ifdef WB_OS_WIN
  return static_cast<int>(::GetLastError());
#else
  return errno;
#endif
}

/**
 * @brief Get system error code.
 * @param rc Native system error code.
 * @return System error code.
 */
[[nodiscard]] inline std::error_code GetSystemErrorCode(
    const int rc = GetLastNativeErrno()) noexcept {
  return std::error_code{rc, std::system_category()};
}

/**
 * @brief System result type.
 * @tparam TResult Result.
*/
template <typename TResult>
using R = std::variant<TResult, std::error_code>;
}  // namespace wb::base

#endif  // !WB_BASE_INCLUDE_SYSTEM_ERROR_EXT_H_
