// Copyright (c) 2023 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Whitebox kernel main world simulation step.

#ifndef WB_KERNEL_MAIN_SIMULATE_STEP_H_
#define WB_KERNEL_MAIN_SIMULATE_STEP_H_

#include <chrono>

#include "hal/drivers/hid/keyboard_input.h"
#include "hal/drivers/hid/mouse_input.h"
#include "kernel/input/input_queue.h"

namespace wb::kernel {

/**
 * @brief Run step world simulation.
 * @param time_delta How much time elapsed since last run?
 * @param mouse_input_queue Mouse input queue.
 * @param keyboard_input_queue Keyboard input queue.
 */
void SimulateWorldStep(
    base::HighResolutionClockDuration time_delta,
    input::InputQueue<hal::hid::MouseInput>& mouse_input_queue,
    input::InputQueue<hal::hid::KeyboardInput>&
        keyboard_input_queue) noexcept;

}  // namespace wb::kernel

#endif  // WB_KERNEL_MAIN_SIMULATE_STEP_H_
