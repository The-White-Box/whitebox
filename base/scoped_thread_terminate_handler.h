// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Scoped handler - thread termination routine to be called by terminate.

#ifndef WB_BASE_SCOPED_THREAD_TERMINATE_HANDLER_H_
#define WB_BASE_SCOPED_THREAD_TERMINATE_HANDLER_H_

#include <eh.h>

#include "base/base_macroses.h"
#include "base/deps/g3log/g3log.h"

namespace wb::base {
/**
 * @brief Changes thread termination routine to be called by terminate.
 */
class ScopedThreadTerminateHandler {
 public:
  /**
   * @brief Set handler when thread terminate called.
   * @param new_handler Handler.
   * @return nothing.
   */
  explicit ScopedThreadTerminateHandler(
      _In_ terminate_function new_terminate_function) noexcept
      : previous_terminate_function_{
            std::set_terminate(new_terminate_function)} {}

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedThreadTerminateHandler);

  /**
   * @brief Restore previous terminate handler.
   */
  ~ScopedThreadTerminateHandler() noexcept {
    std::set_terminate(previous_terminate_function_);
  }

 private:
  /**
   * @brief Previous terminate handler.
   */
  const terminate_function previous_terminate_function_;
};

/**
 * @brief Default thread terminate handler.
 * @return void.
 */
inline void DefaultThreadTerminateHandler() noexcept {
  G3LOG(FATAL) << "Terminate called.  Stopping the app.";
}
}  // namespace wb::base

#endif  // !WB_BASE_SCOPED_THREAD_TERMINATE_HANDLER_H_
