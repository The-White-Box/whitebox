// Copyright (c) 2023 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Whitebox kernel main world simulation step.

#include "kernel/main_simulate_step.h"

namespace wb::kernel {

void SimulateWorldStep(
    base::HighResolutionClockDuration time_delta,
    input::InputQueue<hal::hid::MouseInput>& mouse_input_queue,
    input::InputQueue<hal::hid::KeyboardInput>& keyboard_input_queue) noexcept {
  (void)time_delta;

  // Get input from HID, network, AI.
  // Convert input to world / physics commands.
  // Apply commands to physics world (simulate).
  // Render simulation results (audio, video, force feedback, network, etc.)
  
  // TODO(dimhotepus): Main loop content here.  For now just drain queues.
  auto mouse_input = mouse_input_queue.Pop();
  while (mouse_input.has_value()) {
    mouse_input = mouse_input_queue.Pop();
  }

  auto keyboard_input = keyboard_input_queue.Pop();
  while (keyboard_input.has_value()) {
    keyboard_input = keyboard_input_queue.Pop();
  }
}

}  // namespace wb::kernel
