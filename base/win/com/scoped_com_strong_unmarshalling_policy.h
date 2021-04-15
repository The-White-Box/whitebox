// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// COM unmarshaling allows only a system-trusted list of hardened unmarshalers
// and unmarshalers allowed per-process by the CoAllowUnmarshalerCLSID function.

#ifndef WB_BASE_WIN_COM_SCOPED_COM_STRONG_UNMARSHALLING_POLICY_H_
#define WB_BASE_WIN_COM_SCOPED_COM_STRONG_UNMARSHALLING_POLICY_H_

#include <ObjIdl.h>

#include "base/macroses.h"
#include "base/deps/g3log/g3log.h"
#include "base/win/system_error_ext.h"
#include "com_ptr.h"

namespace wb::base::win::com {

/**
 * @brief Unmarshaling allows only a system-trusted list of hardened
 * unmarshalers and unmarshalers allowed per-process by the
 * CoAllowUnmarshalerCLSID function in scope.
 */
class ScopedComStrongUnmarshallingPolicy {
 public:
  /**
   * @brief Set strict COM unmarshalling policy for scope.
   * @return ScopedComStrongUnmarshallingPolicy.
   */
  static std2::result<ScopedComStrongUnmarshallingPolicy> New() noexcept {
    ScopedComStrongUnmarshallingPolicy policy;

    return !policy.error_code()
               ? std2::result<ScopedComStrongUnmarshallingPolicy>{std::move(
                     policy)}
               : std2::result<ScopedComStrongUnmarshallingPolicy>{
                     policy.error_code()};
  }

  ScopedComStrongUnmarshallingPolicy(
      ScopedComStrongUnmarshallingPolicy&& h) noexcept
      : global_options_{std::move(h.global_options_)},
        error_code_{std::move(h.error_code_)},
        old_global_unmarshalling_policy_option_value_{
            std::move(h.old_global_unmarshalling_policy_option_value_)} {
    // Ensure no deinitialization occurs.
    h.error_code_ = std::error_code{EINVAL, std::generic_category()};
  }

  ScopedComStrongUnmarshallingPolicy& operator=(
      ScopedComStrongUnmarshallingPolicy&&) = delete;

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(ScopedComStrongUnmarshallingPolicy);

  /**
   * @brief Restore old COM unmarshalling policy.
   */
  ~ScopedComStrongUnmarshallingPolicy() noexcept {
    if (!error_code_ && old_global_unmarshalling_policy_option_value_ !=
                            COMGLB_UNMARSHALING_POLICY_STRONG) {
      // Restore old unmarshalling policy.
      G3CHECK(!get_error(
          global_options_->Set(COMGLB_UNMARSHALING_POLICY,
                               old_global_unmarshalling_policy_option_value_)));
    }
  }

 private:
  /**
   * @brief COM global options.
   */
  com_ptr<IGlobalOptions> global_options_;
  /**
   * @brief Error code.
   */
  std::error_code error_code_;
  /**
   * @brief Previous COM unmarshalling policy value.
   */
  ULONG_PTR old_global_unmarshalling_policy_option_value_;

  /**
   * @brief Set strict COM unmarshalling policy for scope.
   * @return nothing.
   */
  ScopedComStrongUnmarshallingPolicy() noexcept
      : global_options_{},
        error_code_{get_error(global_options_.CreateInstance(
            CLSID_GlobalOptions, nullptr, CLSCTX_INPROC_SERVER))},
        old_global_unmarshalling_policy_option_value_{0} {
    if (!error_code_) {
      // Get current COM unmarshalling policy to restore later.
      error_code_ = get_error(global_options_->Query(
          COMGLB_UNMARSHALING_POLICY,
          &old_global_unmarshalling_policy_option_value_));
    }

    if (!error_code_ && old_global_unmarshalling_policy_option_value_ !=
                            COMGLB_UNMARSHALING_POLICY_STRONG) {
      // Unmarshaling allows only a system-trusted list of hardened unmarshalers
      // and unmarshalers allowed per-process by the CoAllowUnmarshalerCLSID
      // function.
      error_code_ = get_error(global_options_->Set(
          COMGLB_UNMARSHALING_POLICY, COMGLB_UNMARSHALING_POLICY_STRONG));
    }

    G3DCHECK(!error_code());
  }

  /**
   * @brief COM unmarshalling policy set result.
   * @return Error code.
   */
  [[nodiscard]] std::error_code error_code() const noexcept {
    return error_code_;
  }
};

}  // namespace wb::base::win::com

#endif  // !WB_BASE_WIN_COM_SCOPED_COM_STRONG_UNMARSHALLING_POLICY_H_
