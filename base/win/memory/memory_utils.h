// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Windows memory utilities.

#ifndef WB_BASE_WIN_MEMORY_MEMORY_UTILS_H_
#define WB_BASE_WIN_MEMORY_MEMORY_UTILS_H_

#include <system_error>

#include "base/config.h"

namespace wb::base::win::memory {

/**
 * @brief Enable process termination on heap corruption.
 * @return Error code.
 */
[[nodiscard]] WB_BASE_API std::error_code
EnableTerminationOnHeapCorruption() noexcept;

/**
 * @brief All heaps in the process with a low-fragmentation heap (LFH) will have
 * their caches optimized, and the memory will be decommitted if possible.
 * @return Error code.
 */
[[nodiscard]] WB_BASE_API std::error_code OptimizeHeapResourcesNow() noexcept;

}  // namespace wb::base::win::memory

#endif  // !WB_BASE_WIN_MEMORY_MEMORY_UTILS_H_
