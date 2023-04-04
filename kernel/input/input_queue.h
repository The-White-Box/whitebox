// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Whitebox input queue.

#ifndef WB_KERNEL_INPUT_INPUT_QUEUE_H_
#define WB_KERNEL_INPUT_INPUT_QUEUE_H_

#include <chrono>
#include <optional>
#include <queue>

#include "base/high_resolution_clock.h"
#include "base/macroses.h"
#include "kernel/input/input_event.h"

namespace wb::kernel::input {

/**
 * @brief Input queue.
 * @tparam T Input.
 */
template <typename T>
class InputQueue {
 public:
  /**
   * @brief Default ctor.
   */
  InputQueue() noexcept : queue_{} {}
  InputQueue(InputQueue<T> &&) noexcept = default;
  InputQueue &operator=(InputQueue<T> &&) noexcept = default;

  /**
   * @brief Emplaces input event.
   * @tparam Args
   */
  template <typename... Args>
  InputEvent<T> &Emplace(InputTimePoint create_time, Args &&...args) noexcept {
    return queue_.emplace(InputEvent{.data = T{std::forward<Args>(args)...},
                                     .create_time = create_time});
  }

  /**
   * @brief Pops first event.
   */
  [[nodiscard]] std::optional<InputEvent<T>> Pop() noexcept {
    if (!queue_.empty()) {
      auto &event = queue_.front();
      queue_.pop();

      return event;
    }

    return std::nullopt;
  }

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(InputQueue);

 private:
  /**
   * @brief Input event queue.
   */
  std::queue<InputEvent<T>> queue_;
};

}  // namespace wb::kernel::input

#endif  // !WB_KERNEL_INPUT_INPUT_QUEUE_H_