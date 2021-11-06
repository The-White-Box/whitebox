// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Scoped handler when pure virtual function call is detected.

#ifndef WB_BASE_WIN_ERROR_HANDLING_SCOPED_PROCESS_PURE_CALL_HANDLER_H_
#define WB_BASE_WIN_ERROR_HANDLING_SCOPED_PROCESS_PURE_CALL_HANDLER_H_

#include <cstdlib>

#include "base/deps/g3log/g3log.h"
#include "build/compiler_config.h"

namespace wb::base::win::error_handling {

/**
 * @brief Set a function to be called when pure virtual function call is
 * detected and reverts back when out of scope.  Because there is one
 * _purecall_handler for the whole process, calling this function immediately
 * impacts all threads.  The last caller on any thread sets the handler.
 */
class ScopedProcessPureCallHandler {
 public:
  WB_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()
    // Pointer or reference to potentially throwing function passed to 'extern
    // "C"' function under -EHc. Undefined behavior may occur if this function
    // throws an exception.  This function should not throw.
    WB_MSVC_DISABLE_WARNING(5039)
    /**
     * @brief Set pure call handler in scope.
     * @param pure_call_handler New pure call handler.
     * @return nothing.
     */
    explicit ScopedProcessPureCallHandler(
        _In_ _purecall_handler pure_call_handler) noexcept
        : old_pure_call_handler_{::_set_purecall_handler(pure_call_handler)} {
      G3DCHECK(!!pure_call_handler);
    }
  WB_MSVC_END_WARNING_OVERRIDE_SCOPE()

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedProcessPureCallHandler);

  /**
   * @brief Restore previous pure call handler.
   */
  ~ScopedProcessPureCallHandler() noexcept {
    WB_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()
      // Pointer or reference to potentially throwing function passed to 'extern
      // "C"' function under -EHc. Undefined behavior may occur if this function
      // throws an exception.  This function should not throw.
      WB_MSVC_DISABLE_WARNING(5039)
      ::_set_purecall_handler(old_pure_call_handler_);
    WB_MSVC_END_WARNING_OVERRIDE_SCOPE()
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

}  // namespace wb::base::win::error_handling

#endif  // !WB_BASE_WIN_ERROR_HANDLING_SCOPED_PROCESS_PURE_CALL_HANDLER_H_
