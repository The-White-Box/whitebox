// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Scoped handler - thread unexpected routine to be called by unexpected.

#ifndef WB_BASE_INCLUDE_WINDOWS_SCOPED_THREAD_UNEXPECTED_HANDLER_H_
#define WB_BASE_INCLUDE_WINDOWS_SCOPED_THREAD_UNEXPECTED_HANDLER_H_
#ifdef _WIN32
#pragma once
#endif

#include <eh.h>

#include "base/include/base_macroses.h"
#include "base/include/deps/g3log/g3log.h"

namespace wb::base::windows {
/**
 * @brief Changes thread unexpected routine to be called by unexpected.
 */
class ScopedThreadUnexpectedHandler {
 public:
  /**
   * @brief Sets handler when thread unexpected called.
   * @param new_handler Handler.
   * @return nothing.
   */
  explicit ScopedThreadUnexpectedHandler(
      _In_ unexpected_function new_unexpected_function) noexcept
      : previous_unexpected_function_{::set_unexpected(new_unexpected_function)} {
  }

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedThreadUnexpectedHandler);

  ~ScopedThreadUnexpectedHandler() noexcept {
    (void)::set_unexpected(previous_unexpected_function_);
  }

 private:
  const unexpected_function previous_unexpected_function_;
};

/**
 * @brief Default thread unexpected handler.
 * @return void.
 */
[[noreturn]] inline void DefaultThreadUnexpectedHandler() noexcept {
  LOG(FATAL) << "Unexpected called.  Stopping the app.";
}
}  // namespace wb::base::windows

#endif  // !WB_BASE_INCLUDE_WINDOWS_SCOPED_THREAD_UNEXPECTED_HANDLER_H_
