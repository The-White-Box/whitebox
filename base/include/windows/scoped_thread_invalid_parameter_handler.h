// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Scoped handler when CRT detects an invalid argument.

#ifndef WHITEBOX_BASE_INCLUDE_WINDOWS_SCOPED_THREAD_INVALID_PARAMETER_HANDLER_H_
#define WHITEBOX_BASE_INCLUDE_WINDOWS_SCOPED_THREAD_INVALID_PARAMETER_HANDLER_H_
#ifdef _WIN32
#pragma once
#endif

#include <cstdlib>

#include "base/include/deps/g3log/g3log.h"

namespace whitebox::base::windows {
/**
 * @brief Sets a function to be called when the CRT detects an invalid argument
 * and reverts back when out of scope.
 */
class ScopedThreadInvalidParameterHandler {
 public:
  /**
   * @brief Sets a function to be called when the CRT detects an invalid
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
    DCHECK(!!invalid_parameter_handler);
  }

  WHITEBOX_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(
      ScopedThreadInvalidParameterHandler);

  ~ScopedThreadInvalidParameterHandler() noexcept {
    (void)::_set_thread_local_invalid_parameter_handler(
        old_invalid_parameter_handler_);
  }

 private:
  const _invalid_parameter_handler old_invalid_parameter_handler_;
};

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
  // Hope this is large enough to not trigger invalid parameter recursion.
  char buffer[1024];
  sprintf_s(buffer, "%S.\nFile: %S (%d)\nExpression: %S.", function, file, line,
            expression);

  LOG(FATAL) << "Invalid parameter detected in function " << buffer
             << " Stopping the app.";
}
}  // namespace whitebox::base::windows

#endif  // !WHITEBOX_BASE_INCLUDE_WINDOWS_SCOPED_THREAD_INVALID_PARAMETER_HANDLER_H_
