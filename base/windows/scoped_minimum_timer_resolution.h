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
#include "base/std_ext/system_error_ext.h"

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
   * @return ScopedMinimumTimerResolution.
   */
  [[nodiscard]] static std::variant<ScopedMinimumTimerResolution, unsigned> New(
      _In_ unsigned minimum_timer_resolution_ms) {
    ScopedMinimumTimerResolution resolution{minimum_timer_resolution_ms};
    return resolution.IsSucceeded()
               ? std::variant<ScopedMinimumTimerResolution, unsigned>{std::move(
                     resolution)}
               : std::variant<ScopedMinimumTimerResolution, unsigned>{
                     resolution.minimum_timer_resolution_error_code_};
  }

  ScopedMinimumTimerResolution(ScopedMinimumTimerResolution &&n) noexcept
      : minimum_timer_resolution_ms_{std::move(n.minimum_timer_resolution_ms_)},
        minimum_timer_resolution_error_code_{
            std::move(n.minimum_timer_resolution_error_code_)} {
    n.minimum_timer_resolution_ms_ = 0;
    n.minimum_timer_resolution_error_code_ = 97;  // TIMERR_NOCANDO
  }
  ScopedMinimumTimerResolution &operator=(
      ScopedMinimumTimerResolution &&) noexcept = delete;

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(ScopedMinimumTimerResolution);

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

 private:
  /**
   * @brief New minimum timer resolution in ms.
   */
  unsigned minimum_timer_resolution_ms_;
  /**
   * @brief Minimum timer resolution creation error_code.
   */
  unsigned minimum_timer_resolution_error_code_;

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

  /**
   * @brief Is set minimum timers resolution succeeded?
   * @return true or false.
   */
  [[nodiscard]] bool IsSucceeded() const noexcept {
    return minimum_timer_resolution_error_code_ == 0;
  }
};
}  // namespace wb::base::windows

#endif  // !WB_BASE_WINDOWS_SCOPED_MINIMUM_TIMER_RESOLUTION_H_
