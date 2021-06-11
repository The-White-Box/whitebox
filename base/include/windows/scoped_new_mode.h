// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Scoped handler when malloc fails to allocate memory.

#ifndef WB_BASE_INCLUDE_WINDOWS_SCOPED_NEW_MODE_H_
#define WB_BASE_INCLUDE_WINDOWS_SCOPED_NEW_MODE_H_
#ifdef _WIN32
#pragma once
#endif

#include <new.h>

#include <type_traits>

#include "base/include/base_macroses.h"

namespace wb::base::windows {
/**
 * @brief Flag determines malloc behaviour on allocation failure.
 */
enum class ScopedNewModeFlag : int {
  /**
   * @brief Doesn't call new.
   */
  DoesntCallNew,
  /**
   * @brief Call new.
   */
  CallNew
};

/**
 * @brief Changes new handler mode for malloc when malloc fails to allocate
 * memory.
 */
class ScopedNewMode {
 public:
  /**
   * @brief Sets a new handler mode for malloc when malloc fails to allocate
   * memory.
   * @param new_mode_flag New mode flag.
   * @return nothing.
   */
  explicit ScopedNewMode(_In_ ScopedNewModeFlag new_mode_flag) noexcept
      : previous_new_mode_{::_set_new_mode(
            static_cast<std::underlying_type_t<decltype(new_mode_flag)>>(
                new_mode_flag))} {}

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedNewMode);

  ~ScopedNewMode() noexcept { (void)::_set_new_mode(previous_new_mode_); }

 private:
  const int previous_new_mode_;
};
}  // namespace wb::base::windows

#endif  // !WB_BASE_INCLUDE_WINDOWS_SCOPED_NEW_HANDLER_H_
