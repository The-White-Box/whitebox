// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Whitebox input event.

#ifndef WB_KERNEL_INPUT_INPUT_EVENT_H_
#define WB_KERNEL_INPUT_INPUT_EVENT_H_

#include <chrono>

#include "base/high_resolution_clock.h"

namespace wb::kernel::input {

/**
 * @brief Input time point.
 */
using InputTimePoint =
	std::chrono::time_point<wb::base::HighResolutionClock>;

/**
 * @brief Input event.
 * @tparam TInput Input.
 */
template <typename TInput>
struct InputEvent {
  /**
   * @brief Input data.
   */
  TInput data;

  /**
   * @brief Input data arrival time.
   */
  InputTimePoint create_time;
};

}  // namespace wb::kernel::input

#endif  // !WB_KERNEL_INPUT_INPUT_EVENT_H_