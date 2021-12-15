// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Scoped handler when new operator fails to allocate memory.

#include "scoped_new_handler.h"

#include <mutex>

#include "internals/scoped_new_handler_internal.h"

namespace wb::base::internals {
/**
 * @brief Mutex to serialize concurrent accesses to global_scoped_new_handler.
 */
std::mutex global_scoped_new_handler_mutex;

/**
 * @brief Global new failure handler.
 */
ScopedNewHandler global_scoped_new_handler;

/**
 * @brief Get global max retries count for the new operator to reallocate
 * memory.
 * @return Global max retries count for the new operator to reallocate memory.
 */
WB_BASE_API std::uint32_t
GetGlobalScopedNewHandlerMaxNewRetriesCount() noexcept {
  std::scoped_lock l{global_scoped_new_handler_mutex};
  return global_scoped_new_handler.max_new_retries_count();
}

}  // namespace wb::base::internals

namespace wb::base {

/**
 * @brief Installs global new failure handler.
 */
WB_BASE_API ScopedNewHandler
InstallGlobalScopedNewHandler(ScopedNewHandler&& handler) noexcept {
  std::scoped_lock l{internals::global_scoped_new_handler_mutex};
  return std::exchange(internals::global_scoped_new_handler,
                       std::move(handler));
}

}  // namespace wb::base
