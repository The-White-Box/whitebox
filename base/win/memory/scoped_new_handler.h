// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Scoped handler when new operator fails to allocate memory.

#ifndef WB_BASE_WIN_MEMORY_SCOPED_NEW_HANDLER_H_
#define WB_BASE_WIN_MEMORY_SCOPED_NEW_HANDLER_H_

#include <new.h>

#include <cerrno>   // ENOMEM.
#include <cstdlib>  // exit.

#include "base/base_macroses.h"
#include "base/deps/g3log/g3log.h"
#include "build/compiler_config.h"

namespace wb::base::win::memory {

/**
 * @brief Changes handler when new operator fails to allocate memory and reverts
 * back when out of scope.  If a user-defined operator new is provided, the new
 * handler functions are not automatically called on failure.
 */
class ScopedNewHandler {
 public:
  WB_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()
    // Pointer or reference to potentially throwing function passed to 'extern
    // "C"' function under -EHc. Undefined behavior may occur if this function
    // throws an exception.  This function should not throw.
    WB_MSVC_DISABLE_WARNING(5039)
    /**
     * @brief Set handler when new operator fails to allocate memory.
     * @param new_handler Handler.
     * @return nothing.
     */
    explicit ScopedNewHandler(_In_ _PNH new_handler) noexcept
        : previous_new_handler_{::_set_new_handler(new_handler)} {}
  WB_MSVC_END_WARNING_OVERRIDE_SCOPE()

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedNewHandler);

  /**
   * @brief Restore previous new handler.
   */
  ~ScopedNewHandler() noexcept {
    WB_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()
      // Pointer or reference to potentially throwing function passed to 'extern
      // "C"' function under -EHc. Undefined behavior may occur if this function
      // throws an exception.  This function should not throw.
      WB_MSVC_DISABLE_WARNING(5039)
      ::_set_new_handler(previous_new_handler_);
    WB_MSVC_END_WARNING_OVERRIDE_SCOPE()
  }

 private:
  /**
   * @brief Previous new handler.
   */
  const _PNH previous_new_handler_;
};

/**
 * @brief The run-time system retries allocation each time your function returns
 * a nonzero value and fails if your function returns 0.
 * @param memory_size_bytes Memory size to allocate.
 * @return Non-0 to retry allocation, 0 on failure to allocate.
 */
[[noreturn]] inline int DefaultNewFailureHandler(size_t memory_size_bytes) {
  G3LOG(FATAL)
      << "Failed to allocate " << memory_size_bytes
      << " memory bytes via new.  Please, ensure you have enough RAM to "
         "run the app.  Stopping the app.";
  // Unreachable.
  std::exit(ENOMEM);
}

}  // namespace wb::base::win::memory

#endif  // !WB_BASE_WIN_MEMORY_SCOPED_NEW_HANDLER_H_
