// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// High resolution clock.

#ifndef WB_BASE_HIGH_RESOLUTION_CLOCK_H_
#define WB_BASE_HIGH_RESOLUTION_CLOCK_H_

#include <chrono>  // std::chrono::steady_clock

namespace wb::base {

/**
 * @brief High resolution clock.  Do not use high_resolution_clock here, as it
 * is not implemented consistently across different standard library
 * implementations, and its use should be avoided.  It is often just an alias
 * for std::chrono::steady_clock or std::chrono::system_clock, but which one it
 * is depends on the library or configuration.
 *
 * Class std::chrono::steady_clock represents a monotonic clock and most
 * suitable for measuring intervals.
 */
using HighResolutionClock = std::chrono::steady_clock;

}  // namespace wb::base

#endif  // !WB_BASE_HIGH_RESOLUTION_CLOCK_H_