// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Set minimum resolution for periodic timers in scope.  Affects Sleep API
// precision, etc.

#ifndef WB_BASE_WINDOWS_SCOPED_MINIMUM_TIMER_RESOLUTION_H_
#define WB_BASE_WINDOWS_SCOPED_MINIMUM_TIMER_RESOLUTION_H_

#include <sal.h>          // _Check_return_
#include <specstrings.h>  // _Post_equals_last_error_

#include "base/base_macroses.h"
#include "base/deps/g3log/g3log.h"

extern "C" _Return_type_success_(
    return == 0) unsigned __stdcall timeBeginPeriod(_In_ unsigned uPeriod);
extern "C" _Return_type_success_(return == 0) unsigned __stdcall timeEndPeriod(
    _In_ unsigned uPeriod);

namespace wb::base::windows {
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
    G3DCHECK(IsSucceeded());
  }

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedMinimumTimerResolution);

  /**
   * @brief Restores previous minimum timer resolution.
   */
  ~ScopedMinimumTimerResolution() noexcept {
    if (IsSucceeded()) {
      [[maybe_unused]] const unsigned rc{
          ::timeEndPeriod(minimum_timer_resolution_ms_)};
      G3CHECK(rc == 0);
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
  /**
   * @brief New minimum timer resolution in ms.
   */
  const unsigned minimum_timer_resolution_ms_;
  /**
   * @brief Minimum timer resolution creation error_code.
   */
  const unsigned minimum_timer_resolution_error_code_;
};
}  // namespace wb::base::windows

#endif  // !WB_BASE_WINDOWS_SCOPED_MINIMUM_TIMER_RESOLUTION_H_
