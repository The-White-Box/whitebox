// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Controls COM fatal exception handling behavior.

#ifndef WB_BASE_WINDOWS_COM_SCOPED_COM_FATAL_EXCEPTION_HANDLER_H_
#define WB_BASE_WINDOWS_COM_SCOPED_COM_FATAL_EXCEPTION_HANDLER_H_

#include <ObjIdl.h>

#include "base/base_macroses.h"
#include "base/deps/g3log/g3log.h"
#include "base/windows/system_error_ext.h"
#include "com_ptr.h"

namespace wb::base::windows::com {
/**
 * @brief Do not handle COM fatal exception, but return to the caller in scope.
 */
class ScopedComFatalExceptionHandler {
 public:
  /**
   * @brief Change COM fatal exception handling in scope.
   * @return nothing.
   */
  ScopedComFatalExceptionHandler() noexcept
      : global_options_{},
        error_code_{GetErrorCode(global_options_.CreateInstance(
            CLSID_GlobalOptions, nullptr, CLSCTX_INPROC_SERVER))},
        old_global_exception_handling_option_value_{0} {
    if (!error_code_) {
      // Get current COM exception handling policy to restore later.
      error_code_ = GetErrorCode(
          global_options_->Query(COMGLB_EXCEPTION_HANDLING,
                                 &old_global_exception_handling_option_value_));
    }

    if (!error_code_ && old_global_exception_handling_option_value_ !=
                            COMGLB_EXCEPTION_DONOT_HANDLE_ANY) {
      // When set and a fatal exception occurs in a COM method, this causes the
      // COM runtime to not handle the exception.
      error_code_ = GetErrorCode(global_options_->Set(
          COMGLB_EXCEPTION_HANDLING, COMGLB_EXCEPTION_DONOT_HANDLE_ANY));
    }

    G3DCHECK(!error_code());
  }

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedComFatalExceptionHandler);

  /**
   * @brief REstore previous COM global options.
   */
  ~ScopedComFatalExceptionHandler() noexcept {
    // Well, we can't restore old COM exception handling policy, it fails with
    // E_FAIL!  So decided to fallback to COMGLB_EXCEPTION_DONOT_HANDLE as a
    // less restricted one.
    if (!error_code_ && old_global_exception_handling_option_value_ !=
                            COMGLB_EXCEPTION_DONOT_HANDLE_ANY) {
      // Restore "old" exception handling policy.
      G3CHECK(!GetErrorCode(global_options_->Set(
          COMGLB_EXCEPTION_HANDLING, COMGLB_EXCEPTION_DONOT_HANDLE)));
    }
  }

  /**
   * @brief Error code.
   * @return Error code.
   */
  [[nodiscard]] std::error_code error_code() const noexcept {
    return error_code_;
  }

 private:
  /**
   * @brief Global COM options.
   */
  com_ptr<IGlobalOptions> global_options_;
  /**
   * @brief Error code.
   */
  std::error_code error_code_;
  /**
   * @brief Previous COM global options exception handling value.
   */
  ULONG_PTR old_global_exception_handling_option_value_;
};
}  // namespace wb::base::windows::com

#endif  // !WB_BASE_WINDOWS_COM_SCOPED_COM_FATAL_EXCEPTION_HANDLER_H_
