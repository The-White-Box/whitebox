// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Scoped the calling thread's last-error code value.

#ifndef WB_BASE_WINDOWS_ERROR_HANDLING_SCOPED_THREAD_LAST_ERROR_H_
#define WB_BASE_WINDOWS_ERROR_HANDLING_SCOPED_THREAD_LAST_ERROR_H_

#include <sal.h>          // _Check_return_
#include <specstrings.h>  // _Post_equals_last_error_

#include "base/base_macroses.h"
#include "build/compiler_config.h"

extern "C" WB_ATTRIBUTE_DLL_IMPORT _Check_return_
    _Post_equals_last_error_ unsigned long __stdcall GetLastError(void);
extern "C" WB_ATTRIBUTE_DLL_IMPORT void __stdcall SetLastError(
    _In_ unsigned long dwErrCode);

namespace wb::base::windows::error_handling {
/**
 * @brief Changes calling thread's last-error code in scope and reverts back
 * when out of scope.
 */
class ScopedThreadLastError {
 public:
  /**
   * @brief Set scoped calling thread's last-error code.
   * @param last_error Last error.
   * @return nothing.
   */
  explicit ScopedThreadLastError(
      unsigned long last_error = ::GetLastError()) noexcept
      : previous_last_error_{last_error} {}

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedThreadLastError);

  /**
   * @brief Restore previous thread last error.
   */
  ~ScopedThreadLastError() noexcept { ::SetLastError(previous_last_error_); }

 private:
  /**
   * @brief Previous thread last error.
   */
  const unsigned long previous_last_error_;
};
}  // namespace wb::base::windows::error_handling

#endif  // !WB_BASE_WINDOWS_ERROR_HANDLING_SCOPED_THREAD_LAST_ERROR_H_
