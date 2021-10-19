// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Scoped handler when malloc fails to allocate memory.

#ifndef WB_BASE_WIN_MEMORY_SCOPED_NEW_MODE_H_
#define WB_BASE_WIN_MEMORY_SCOPED_NEW_MODE_H_

#include <new.h>

#include <type_traits>

#include "base/base_macroses.h"

namespace wb::base::win::memory {

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
   * @brief Set a new handler mode for malloc when malloc fails to allocate
   * memory.
   * @param new_mode_flag New mode flag.
   * @return nothing.
   */
  explicit ScopedNewMode(_In_ ScopedNewModeFlag new_mode_flag) noexcept
      : previous_new_mode_{::_set_new_mode(underlying_cast(new_mode_flag))} {}

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedNewMode);

  /**
   * @brief Restores previous new mode.
   */
  ~ScopedNewMode() noexcept { ::_set_new_mode(previous_new_mode_); }

 private:
  /**
   * @brief Previous new mode.
   */
  const int previous_new_mode_;
};

}  // namespace wb::base::win::memory

#endif  // !WB_BASE_WIN_MEMORY_SCOPED_NEW_MODE_H_
