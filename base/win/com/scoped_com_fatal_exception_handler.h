// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Controls COM fatal exception handling behavior.

#ifndef WB_BASE_WIN_COM_SCOPED_COM_FATAL_EXCEPTION_HANDLER_H_
#define WB_BASE_WIN_COM_SCOPED_COM_FATAL_EXCEPTION_HANDLER_H_

#include <ObjIdl.h>

#include "base/base_macroses.h"
#include "base/deps/g3log/g3log.h"
#include "base/win/system_error_ext.h"
#include "com_ptr.h"

namespace wb::base::win::com {

/**
 * @brief Do not handle COM fatal exception, but return to the caller in scope.
 */
class ScopedComFatalExceptionHandler {
 public:
  /**
   * @brief Change COM fatal exception handling in scope.
   * @return ScopedComFatalExceptionHandler.
   */
  static std2::result<ScopedComFatalExceptionHandler> New() noexcept {
    ScopedComFatalExceptionHandler handler;

    return !handler.error_code()
               ? std2::result<ScopedComFatalExceptionHandler>{std::move(
                     handler)}
               : std2::result<ScopedComFatalExceptionHandler>{
                     handler.error_code()};
  }

  ScopedComFatalExceptionHandler(ScopedComFatalExceptionHandler&& h) noexcept
      : global_options_{std::move(h.global_options_)},
        error_code_{std::move(h.error_code_)},
        old_global_exception_handling_option_value_{
            std::move(h.old_global_exception_handling_option_value_)} {
    // Ensure no deinitialization occurs.
    h.error_code_ = std::error_code{EINVAL, std::generic_category()};
  }

  ScopedComFatalExceptionHandler& operator=(ScopedComFatalExceptionHandler&&) =
      delete;

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(ScopedComFatalExceptionHandler);

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
      G3CHECK(!get_error(global_options_->Set(
          COMGLB_EXCEPTION_HANDLING, COMGLB_EXCEPTION_DONOT_HANDLE)));
    }
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

  /**
   * @brief Change COM fatal exception handling in scope.
   * @return nothing.
   */
  ScopedComFatalExceptionHandler() noexcept
      : global_options_{},
        error_code_{get_error(global_options_.CreateInstance(
            CLSID_GlobalOptions, nullptr, CLSCTX_INPROC_SERVER))},
        old_global_exception_handling_option_value_{0} {
    if (!error_code_) {
      // Get current COM exception handling policy to restore later.
      error_code_ = get_error(
          global_options_->Query(COMGLB_EXCEPTION_HANDLING,
                                 &old_global_exception_handling_option_value_));
    }

    if (!error_code_ && old_global_exception_handling_option_value_ !=
                            COMGLB_EXCEPTION_DONOT_HANDLE_ANY) {
      // When set and a fatal exception occurs in a COM method, this causes the
      // COM runtime to not handle the exception.
      error_code_ = get_error(global_options_->Set(
          COMGLB_EXCEPTION_HANDLING, COMGLB_EXCEPTION_DONOT_HANDLE_ANY));
    }

    G3DCHECK(!error_code());
  }

  /**
   * @brief Error code.
   * @return Error code.
   */
  [[nodiscard]] std::error_code error_code() const noexcept {
    return error_code_;
  }
};

}  // namespace wb::base::win::com

#endif  // !WB_BASE_WIN_COM_SCOPED_COM_FATAL_EXCEPTION_HANDLER_H_
