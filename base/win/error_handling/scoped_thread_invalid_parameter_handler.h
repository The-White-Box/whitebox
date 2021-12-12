// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Scoped handler when CRT detects an invalid argument.

#ifndef WB_BASE_WIN_ERROR_HANDLING_SCOPED_THREAD_INVALID_PARAMETER_HANDLER_H_
#define WB_BASE_WIN_ERROR_HANDLING_SCOPED_THREAD_INVALID_PARAMETER_HANDLER_H_

#include <cstdlib>

#include "base/deps/g3log/g3log.h"
#include "build/compiler_config.h"

namespace wb::base::win::error_handling {

/**
 * @brief Set a function to be called when the CRT detects an invalid argument
 * and reverts back when out of scope.
 */
class ScopedThreadInvalidParameterHandler {
 public:
  WB_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()
    // Pointer or reference to potentially throwing function passed to 'extern
    // "C"' function under -EHc. Undefined behavior may occur if this function
    // throws an exception.  This function should not throw.
    WB_MSVC_DISABLE_WARNING(5039)
    /**
     * @brief Set a function to be called when the CRT detects an invalid
     * argument.
     * @param invalid_parameter_handler New CRT invalid argument handler for
     * thread.
     * @return nothing.
     */
    explicit ScopedThreadInvalidParameterHandler(
        _In_ _invalid_parameter_handler invalid_parameter_handler) noexcept
        : old_invalid_parameter_handler_{
              ::_set_thread_local_invalid_parameter_handler(
                  invalid_parameter_handler)} {
      G3DCHECK(!!invalid_parameter_handler);
    }
  WB_MSVC_END_WARNING_OVERRIDE_SCOPE()

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedThreadInvalidParameterHandler);

  /**
   * @brief Restore previous invalid parameter handler.
   */
  ~ScopedThreadInvalidParameterHandler() noexcept {
    WB_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()
      // Pointer or reference to potentially throwing function passed to 'extern
      // "C"' function under -EHc. Undefined behavior may occur if this function
      // throws an exception.  This function should not throw.
      WB_MSVC_DISABLE_WARNING(5039)
      ::_set_thread_local_invalid_parameter_handler(
          old_invalid_parameter_handler_);
    WB_MSVC_END_WARNING_OVERRIDE_SCOPE()
  }

 private:
  /**
   * @brief Previous invalid parameter handler.
   */
  const _invalid_parameter_handler old_invalid_parameter_handler_;
};

namespace details {

/**
 * @brief Ensure no reentrancy happened into scope.
 */
class ScopedInvalidParameterReentrancyGuard {
 public:
  /**
   * @brief Mark scope as entered.
   * @param is_entered Is scope entered.
   * @return nothing.
   */
  ScopedInvalidParameterReentrancyGuard(bool& is_entered) noexcept
      : is_entered_{is_entered} {
    is_entered = true;
  }

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedInvalidParameterReentrancyGuard);

  /**
   * @brief Mark scope not entered.
   */
  ~ScopedInvalidParameterReentrancyGuard() noexcept { is_entered_ = false; }

 private:
  /**
   * @brief Is scope reentered.
   */
  bool& is_entered_;
};

}  // namespace details

/**
 * @brief Default CRT invalid parameter handler.
 * @param expression Expression which caused failure.
 * @param function Function which caused failure.
 * @param file File.
 * @param line Line.
 * @param  Context.
 */
[[noreturn]] inline void DefaultThreadInvalidParameterHandler(
    const wchar_t* expression, const wchar_t* function, const wchar_t* file,
    unsigned int line, uintptr_t) {
  static bool is_entered_handler{false};
  // Hope this is large enough to not trigger invalid parameter recursively.
  char buffer[512];

  if (!is_entered_handler) {
    // Ensure we do not invoke this scope recursively.
    details::ScopedInvalidParameterReentrancyGuard
        ensure_not_reenter_this_scope{is_entered_handler};

    // Can fail with invalid parameter again, so guard below.
    sprintf_s(buffer, "%S.\nFile: %S (%u)\nExpression: %S.", function, file,
              line, expression);

    G3LOG(FATAL) << "Invalid parameter detected in function " << buffer
                 << " Stopping the app.";
  } else {
    sprintf_s(buffer, "%S.", function);

    // Oops, recursive reenter, means invalid parameter error in handling scope.
    G3LOG(FATAL) << "Invalid parameter detected during invalid parameter "
                    "handling in function "
                 << buffer << " Stopping the app.";
  }
}

}  // namespace wb::base::win::error_handling

#endif  // !WB_BASE_WIN_ERROR_HANDLING_SCOPED_THREAD_INVALID_PARAMETER_HANDLER_H_
