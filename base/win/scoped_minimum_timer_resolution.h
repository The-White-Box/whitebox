// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Set minimum resolution for periodic timers in scope.  Affects Sleep API
// precision, etc.

#ifndef WB_BASE_WIN_SCOPED_MINIMUM_TIMER_RESOLUTION_H_
#define WB_BASE_WIN_SCOPED_MINIMUM_TIMER_RESOLUTION_H_

#include <sal.h>          // _Check_return_
#include <specstrings.h>  // _Post_equals_last_error_

#include <chrono>
#include <cstddef>  // std::byte

#include "base/base_macroses.h"
#include "base/deps/g3log/g3log.h"
#include "base/std2/system_error_ext.h"

extern "C" _Return_type_success_(
    return == 0) unsigned __stdcall timeBeginPeriod(_In_ unsigned uPeriod);
extern "C" _Return_type_success_(return == 0) unsigned __stdcall timeEndPeriod(
    _In_ unsigned uPeriod);

namespace wb::base::windows {
/**
 * @brief Changes minimum resolution for periodic timers and reverts back when
 * out of scope.
 *
 * "Prior to Windows 10, version 2004, this function affects a global Windows
 * setting.  For all processes Windows uses the lowest value (that is, highest
 * resolution) requested by any process.  Starting with Windows 10, version
 * 2004, this function no longer affects global timer resolution.  For processes
 * which call this function, Windows uses the lowest value (that is, highest
 * resolution) requested by any process.  For processes which have not called
 * this function, Windows does not guarantee a higher resolution than the
 * default system resolution.
 *
 * Starting with Windows 11, if a window-owning process becomes fully occluded,
 * minimized, or otherwise invisible or inaudible to the end user, Windows does
 * not guarantee a higher resolution than the default system resolution.  See
 * SetProcessInformation for more information on this behavior.
 *
 * Setting a higher resolution can improve the accuracy of time-out intervals in
 * wait functions.  However, it can also reduce overall system performance,
 * because the thread scheduler switches tasks more often.  High resolutions can
 * also prevent the CPU power management system from entering power-saving
 * modes.  Setting a higher resolution does not improve the accuracy of the
 * high-resolution performance counter."
 *
 * See
 * https://docs.microsoft.com/en-us/windows/win32/api/timeapi/nf-timeapi-timebeginperiod
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
      _In_ std::chrono::milliseconds minimum_timer_resolution_ms) {
    ScopedMinimumTimerResolution resolution{minimum_timer_resolution_ms};
    return resolution.is_succeeded()
               ? std::variant<ScopedMinimumTimerResolution, unsigned>{std::move(
                     resolution)}
               : std::variant<ScopedMinimumTimerResolution, unsigned>{
                     resolution.minimum_timer_resolution_error_code_};
  }

  ScopedMinimumTimerResolution(ScopedMinimumTimerResolution &&n) noexcept
      : minimum_timer_resolution_ms_{std::move(n.minimum_timer_resolution_ms_)},
        minimum_timer_resolution_error_code_{
            std::move(n.minimum_timer_resolution_error_code_)} {
    using namespace std::chrono_literals;

    n.minimum_timer_resolution_ms_ = 0ms;
    n.minimum_timer_resolution_error_code_ = 97;  // TIMERR_NOCANDO
  }
  ScopedMinimumTimerResolution &operator=(
      ScopedMinimumTimerResolution &&) noexcept = delete;

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(ScopedMinimumTimerResolution);

  /**
   * @brief Restores previous minimum timer resolution.
   */
  ~ScopedMinimumTimerResolution() noexcept {
    if (is_succeeded()) {
      G3DCHECK(minimum_timer_resolution_ms_.count() <=
               static_cast<decltype(minimum_timer_resolution_ms_)::rep>(
                   std::numeric_limits<unsigned>::max()))
          << "Resolution is truncated before pass to timeEndPeriod.";
      [[maybe_unused]] const unsigned rc{::timeEndPeriod(
          static_cast<unsigned>(minimum_timer_resolution_ms_.count()))};
      G3CHECK(rc == 0);
    }
  }

 private:
  /**
   * @brief New minimum timer resolution in ms.
   */
  std::chrono::milliseconds minimum_timer_resolution_ms_;
  /**
   * @brief Minimum timer resolution creation error_code.
   */
  unsigned minimum_timer_resolution_error_code_;

  [[maybe_unused]] std::byte
      pad_[sizeof(char *) - sizeof(minimum_timer_resolution_error_code_)];

  /**
   * @brief Changes minimum resolution for periodic timers.
   * @param minimum_timer_resolution_ms Minimum timers resolution in
   * milliseconds to request.
   * @return nothing.
   */
  explicit ScopedMinimumTimerResolution(
      _In_ std::chrono::milliseconds minimum_timer_resolution_ms) noexcept
      : minimum_timer_resolution_ms_{minimum_timer_resolution_ms},
        minimum_timer_resolution_error_code_{
            ::timeBeginPeriod(static_cast<unsigned>(minimum_timer_resolution_ms.count()))} {
    G3CHECK(minimum_timer_resolution_ms.count() <=
            static_cast<decltype(minimum_timer_resolution_ms)::rep>(
                std::numeric_limits<unsigned>::max()))
        << "Resolution is truncated before pass to timeBeginPeriod.";
    G3DCHECK(is_succeeded());
  }

  /**
   * @brief Is set minimum timers resolution succeeded?
   * @return true or false.
   */
  [[nodiscard]] bool is_succeeded() const noexcept {
    return minimum_timer_resolution_error_code_ == 0;
  }
};
}  // namespace wb::base::windows

#endif  // !WB_BASE_WIN_SCOPED_MINIMUM_TIMER_RESOLUTION_H_
