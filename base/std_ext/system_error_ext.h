// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// <system_error> extensions.

#ifndef WB_BASE_STD_EXT_SYSTEM_ERROR_EXT_H_
#define WB_BASE_STD_EXT_SYSTEM_ERROR_EXT_H_

#include <system_error>
#include <variant>

#include "build/build_config.h"
#include "build/compiler_config.h"

#ifdef WB_OS_WIN
#include <sal.h>          // _Check_return_
#include <specstrings.h>  // _Post_equals_last_error_

extern "C" WB_ATTRIBUTE_DLL_IMPORT _Check_return_
    _Post_equals_last_error_ unsigned long __stdcall GetLastError(void);
extern "C" WB_ATTRIBUTE_DLL_IMPORT void __stdcall SetLastError(
    _In_ unsigned long dwErrCode);
#else
#include <cerrno>
#endif

namespace wb::base::std_ext {
/**
 * @brief Get last native system errno.
 * @return Last native system errno.
 */
[[nodiscard]] inline int GetThreadNativeLastErrno() noexcept {
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
[[nodiscard]] inline std::error_code GetThreadErrorCode(
    const int rc = GetThreadNativeLastErrno()) noexcept {
  return std::error_code{rc, std::system_category()};
}

/**
 * @brief Set system error code.
 * @param rc Native system error code.
 */
[[nodiscard]] inline void SetThreadErrorCode(
    const std::error_code rc) noexcept {
#ifdef WB_OS_WIN
  ::SetLastError(static_cast<unsigned long>(rc.value()));
#else
  errno = rc.value();
#endif
}

/**
 * @brief Get generic POSIX errno error code.
 * @param rc Native POSIX errno error code.
 * @return POSIX errno error code.
 */
[[nodiscard]] inline std::error_code GetThreadPosixErrorCode(
    const int rc = GetThreadNativeLastErrno()) noexcept {
  return std::error_code{rc, std::generic_category()};
}

/**
 * @brief System result type.
 * @tparam TResult Result.
 */
template <typename TResult>
using os_res = std::variant<TResult, std::error_code>;
}  // namespace wb::base::std_ext

#endif  // !WB_BASE_STD_EXT_SYSTEM_ERROR_EXT_H_
