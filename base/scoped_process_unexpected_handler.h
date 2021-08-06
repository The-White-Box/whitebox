// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Scoped handler - process unexpected routine to be called by unexpected.

#ifndef WB_BASE_SCOPED_PROCESS_UNEXPECTED_HANDLER_H_
#define WB_BASE_SCOPED_PROCESS_UNEXPECTED_HANDLER_H_

#include "build/build_config.h"

#if defined(WB_OS_POSIX)
#include <exception>

#define WB_UNEXPECTED_HANDLER_NAMESPACE std::
#elif defined(WB_OS_WIN)
#include <eh.h>

#define WB_UNEXPECTED_HANDLER_NAMESPACE
#endif

#include "base/base_macroses.h"
#include "base/deps/g3log/g3log.h"

namespace wb::base {
/**
 * @brief Changes process unexpected routine to be called by unexpected.
 */
class ScopedProcessUnexpectedHandler {
 public:
  /**
   * @brief Set handler when process unexpected called.
   * @param new_handler Handler.
   * @return nothing.
   */
  explicit ScopedProcessUnexpectedHandler(
      WB_UNEXPECTED_HANDLER_NAMESPACE unexpected_handler
          new_unexpected_handler) noexcept
      : previous_unexpected_handler_{
            WB_UNEXPECTED_HANDLER_NAMESPACE set_unexpected(
                new_unexpected_handler)} {}

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedProcessUnexpectedHandler);

  /**
   * @brief Restore previous unexpected thread handler.
   */
  ~ScopedProcessUnexpectedHandler() noexcept {
    WB_UNEXPECTED_HANDLER_NAMESPACE set_unexpected(
        previous_unexpected_handler_);
  }

 private:
  /**
   * @brief Previous unexpected thread handler.
   */
  const WB_UNEXPECTED_HANDLER_NAMESPACE unexpected_handler
      previous_unexpected_handler_;
};

/**
 * @brief Default thread unexpected handler.
 * @return void.
 */
inline void DefaultProcessUnexpectedHandler() noexcept {
  G3LOG(FATAL) << "Unexpected called.  Stopping the app.";
}
}  // namespace wb::base

#endif  // !WB_BASE_SCOPED_PROCESS_UNEXPECTED_HANDLER_H_
