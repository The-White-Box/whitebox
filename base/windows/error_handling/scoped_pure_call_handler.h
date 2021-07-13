// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Scoped handler when pure virtual function call is detected.

#ifndef WB_BASE_WINDOWS_ERROR_HANDLING_SCOPED_PURE_CALL_HANDLER_H_
#define WB_BASE_WINDOWS_ERROR_HANDLING_SCOPED_PURE_CALL_HANDLER_H_

#include <cstdlib>

#include "base/deps/g3log/g3log.h"

namespace wb::base::windows::error_handling {
/**
 * @brief Set a function to be called when pure virtual function call is
 * detected and reverts back when out of scope.
 */
class ScopedPureCallHandler {
 public:
  /**
   * @brief Set pure call handler in scope.
   * @param pure_call_handler New pure call handler.
   * @return nothing.
   */
  explicit ScopedPureCallHandler(
      _In_ _purecall_handler pure_call_handler) noexcept
      : old_pure_call_handler_{::_set_purecall_handler(pure_call_handler)} {
    G3DCHECK(!!pure_call_handler);
  }

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedPureCallHandler);

  /**
   * @brief Restore previous pure call handler.
   */
  ~ScopedPureCallHandler() noexcept {
    ::_set_purecall_handler(old_pure_call_handler_);
  }

 private:
  /**
   * @brief Previous pure call handler.
   */
  const _purecall_handler old_pure_call_handler_;
};

/**
 * @brief Default pure call handler.
 */
[[noreturn]] inline void DefaultPureCallHandler() {
  G3LOG(FATAL) << "Pure virtual function call occured.  Stopping the app.";
}
}  // namespace wb::base::windows::error_handling

#endif  // !WB_BASE_WINDOWS_ERROR_HANDLING_SCOPED_PURE_CALL_HANDLER_H_
