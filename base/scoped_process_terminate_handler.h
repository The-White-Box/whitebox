// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Scoped handler - process termination routine to be called by terminate.

#ifndef WB_BASE_SCOPED_PROCESS_TERMINATE_HANDLER_H_
#define WB_BASE_SCOPED_PROCESS_TERMINATE_HANDLER_H_

#include <exception>

#include "base/base_macroses.h"
#include "base/deps/g3log/g3log.h"
#include "build/compiler_config.h"

namespace wb::base {

/**
 * @brief Changes process termination routine to be called by terminate.
 */
class ScopedProcessTerminateHandler {
 public:
  WB_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()
    // Pointer or reference to potentially throwing function passed to 'extern
    // "C"' function under -EHc. Undefined behavior may occur if this function
    // throws an exception.  This function should not throw.
    WB_MSVC_DISABLE_WARNING(5039)
    /**
     * @brief Set handler when process terminate called.
     * @param new_handler Handler.
     * @return nothing.
     */
    explicit ScopedProcessTerminateHandler(
        std::terminate_handler new_terminate_function) noexcept
        : previous_terminate_function_{
              std::set_terminate(new_terminate_function)} {}
  WB_MSVC_END_WARNING_OVERRIDE_SCOPE()

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedProcessTerminateHandler);

  /**
   * @brief Restore previous terminate handler.
   */
  ~ScopedProcessTerminateHandler() noexcept {
    WB_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()
      // Pointer or reference to potentially throwing function passed to 'extern
      // "C"' function under -EHc. Undefined behavior may occur if this function
      // throws an exception.  This function should not throw.
      WB_MSVC_DISABLE_WARNING(5039)
      std::set_terminate(previous_terminate_function_);
    WB_MSVC_END_WARNING_OVERRIDE_SCOPE()
  }

 private:
  /**
   * @brief Previous terminate handler.
   */
  const std::terminate_handler previous_terminate_function_;
};

/**
 * @brief Default process terminate handler.
 * @return void.
 */
inline void DefaultProcessTerminateHandler() noexcept {
  G3LOG(FATAL) << "Terminate called.  Stopping the app.";
}

}  // namespace wb::base

#endif  // !WB_BASE_SCOPED_PROCESS_TERMINATE_HANDLER_H_
