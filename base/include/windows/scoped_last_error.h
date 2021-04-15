// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Scoped last system error.

#ifndef WHITEBOX_BASE_INCLUDE_WINDOWS_SCOPED_LAST_ERROR_H_
#define WHITEBOX_BASE_INCLUDE_WINDOWS_SCOPED_LAST_ERROR_H_
#ifdef _WIN32
#pragma once
#endif

#include <sal.h>          // _Check_return_
#include <specstrings.h>  // _Post_equals_last_error_

#include "base/include/base_macroses.h"

extern "C" __declspec(dllimport) _Check_return_ _Post_equals_last_error_
    unsigned long __stdcall GetLastError(void);
extern "C" __declspec(dllimport) void __stdcall SetLastError(
    _In_ unsigned long dwErrCode);

namespace whitebox::base::windows {
/**
 * @brief Changes last error in scope and reverts back when out of scope.
 */
class ScopedLastError {
 public:
  /**
   * @brief Sets scoped last error.
   * @param last_error Last error.
   * @return nothing.
   */
  explicit ScopedLastError(unsigned long last_error = ::GetLastError()) noexcept
      : previous_last_error_{last_error} {}

  WHITEBOX_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedLastError);

  ~ScopedLastError() noexcept { ::SetLastError(previous_last_error_); }

 private:
  const unsigned long previous_last_error_;
};
}  // namespace whitebox::base::windows

#endif  // !WHITEBOX_BASE_INCLUDE_WINDOWS_SCOPED_LAST_ERROR_H_
