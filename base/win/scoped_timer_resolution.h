// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Set minimum resolution for periodic timers in scope.  Affects Sleep / Wait
// API precision, etc.

#ifndef WB_BASE_WIN_SCOPED_TIMER_RESOLUTION_H_
#define WB_BASE_WIN_SCOPED_TIMER_RESOLUTION_H_

#include <sal.h>          // _Check_return_
#include <specstrings.h>  // _Post_equals_last_error_

#include <chrono>
#include <cstddef>  // std::byte

#include "base/base_macroses.h"
#include "base/deps/g3log/g3log.h"
#include "base/std2/system_error_ext.h"

extern "C" WB_ATTRIBUTE_DLL_IMPORT _Return_type_success_(
    return == 0) unsigned __stdcall timeBeginPeriod(_In_ unsigned uPeriod);
extern "C" WB_ATTRIBUTE_DLL_IMPORT _Return_type_success_(
    return == 0) unsigned __stdcall timeEndPeriod(_In_ unsigned uPeriod);

namespace wb::base::win {

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
class ScopedTimerResolution {
 public:
  /**
   * @brief ScopedTimerResolution creation result.
   */
  using NewResult = std::variant<ScopedTimerResolution, unsigned>;

  /**
   * @brief Changes minimum resolution for periodic timers.
   * @param resolution_ms Minimum timers resolution in
   * milliseconds to request.
   * @return ScopedTimerResolution.
   */
  [[nodiscard]] static NewResult New(
      _In_ std::chrono::milliseconds resolution_ms) {
    ScopedTimerResolution resolution{resolution_ms};
    return resolution.is_succeeded() ? NewResult{std::move(resolution)}
                                     : NewResult{resolution.error_code_};
  }

  ScopedTimerResolution(ScopedTimerResolution &&n) noexcept
      : resolution_ms_{std::move(n.resolution_ms_)},
        error_code_{std::move(n.error_code_)} {
    using namespace std::chrono_literals;

    n.resolution_ms_ = 0ms;
    n.error_code_ = 97;  // TIMERR_NOCANDO
  }
  ScopedTimerResolution &operator=(ScopedTimerResolution &&) noexcept = delete;

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(ScopedTimerResolution);

  /**
   * @brief Restores previous minimum timer resolution.
   */
  ~ScopedTimerResolution() noexcept {
    if (is_succeeded()) {
      G3DCHECK(resolution_ms_.count() <=
               static_cast<decltype(resolution_ms_)::rep>(
                   std::numeric_limits<unsigned>::max()))
          << "Resolution is truncated before pass to timeEndPeriod.";

      G3CHECK(::timeEndPeriod(static_cast<unsigned>(resolution_ms_.count())) ==
              0);
    }
  }

 private:
  /**
   * @brief New minimum timer resolution in ms.
   */
  std::chrono::milliseconds resolution_ms_;
  /**
   * @brief Minimum timer resolution creation error_code.
   */
  unsigned error_code_;

  WB_ATTRIBUTE_UNUSED_FIELD std::byte
      pad_[sizeof(char *) - sizeof(error_code_)];

  /**
   * @brief Changes minimum resolution for periodic timers.
   * @param resolution_ms Minimum timers resolution in
   * milliseconds to request.
   * @return nothing.
   */
  explicit ScopedTimerResolution(
      _In_ std::chrono::milliseconds resolution_ms) noexcept
      : resolution_ms_{resolution_ms},
        error_code_{
            ::timeBeginPeriod(static_cast<unsigned>(resolution_ms.count()))} {
    G3CHECK(resolution_ms.count() <= static_cast<decltype(resolution_ms)::rep>(
                                         std::numeric_limits<unsigned>::max()))
        << "Resolution is truncated before pass to timeBeginPeriod.";
    G3DCHECK(is_succeeded());
  }

  /**
   * @brief Is set minimum timers resolution succeeded?
   * @return true or false.
   */
  [[nodiscard]] bool is_succeeded() const noexcept { return error_code_ == 0; }
};

}  // namespace wb::base::win

#endif  // !WB_BASE_WIN_SCOPED_TIMER_RESOLUTION_H_
