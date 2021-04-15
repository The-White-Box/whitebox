// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Scoped minimum resolution for periodic timers setter.

#ifndef WHITEBOX_BASE_INCLUDE_WINDOWS_SCOPED_MINIMUM_TIMER_RESOLUTION_H_
#define WHITEBOX_BASE_INCLUDE_WINDOWS_SCOPED_MINIMUM_TIMER_RESOLUTION_H_
#ifdef _WIN32
#pragma once
#endif

#include <sal.h>          // _Check_return_
#include <specstrings.h>  // _Post_equals_last_error_

#include "base/include/base_macroses.h"
#include "base/include/deps/g3log/g3log.h"

extern "C" _Return_type_success_(
    return == 0) unsigned __stdcall timeBeginPeriod(_In_ unsigned uPeriod);
extern "C" _Return_type_success_(return == 0) unsigned __stdcall timeEndPeriod(
    _In_ unsigned uPeriod);

namespace whitebox::base::windows {
/**
 * @brief Changes minimum resolution for periodic timers and reverts back when
 * out of scope.
 */
class ScopedMinimumTimerResolution {
 public:
  /**
   * @brief Changes minimum resolution for periodic timers.
   * @param minimum_timer_resolution_ms Minimum timers resolution in
   * milliseconds to request.
   * @return nothing.
   */
  explicit ScopedMinimumTimerResolution(
      _In_ unsigned minimum_timer_resolution_ms) noexcept
      : minimum_timer_resolution_ms_{minimum_timer_resolution_ms},
        minimum_timer_resolution_error_code_{
            ::timeBeginPeriod(minimum_timer_resolution_ms)} {
    DCHECK(IsSucceeded());
  }

  WHITEBOX_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedMinimumTimerResolution);

  ~ScopedMinimumTimerResolution() noexcept {
    if (IsSucceeded()) {
      [[maybe_unused]] const unsigned rc{
          ::timeEndPeriod(minimum_timer_resolution_ms_)};
      DCHECK(rc == 0);
    }
  }

  /**
   * @brief Is set minimum timers resolution succeeded?
   * @return true or false.
   */
  [[nodiscard]] bool IsSucceeded() const noexcept {
    return minimum_timer_resolution_error_code_ == 0;
  }

 private:
  const unsigned minimum_timer_resolution_ms_;
  const unsigned minimum_timer_resolution_error_code_;
};
}  // namespace whitebox::base::windows

#endif  // !WHITEBOX_BASE_INCLUDE_WINDOWS_SCOPED_MINIMUM_TIMER_RESOLUTION_H_
