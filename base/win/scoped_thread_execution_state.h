// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Enables an application to inform the system that it is in use, thereby
// preventing the system from entering sleep or turning off the display while
// the application is running.

#ifndef WB_BASE_WIN_SCOPED_THREAD_EXECUTION_STATE_H_
#define WB_BASE_WIN_SCOPED_THREAD_EXECUTION_STATE_H_

#include <array>
#include <cstddef>  // std::byte

#include "base/macroses.h"
#include "base/deps/g3log/g3log.h"
#include "base/std2/system_error_ext.h"

using EXECUTION_STATE = unsigned long;

extern "C" WB_ATTRIBUTE_DLL_IMPORT
    EXECUTION_STATE __stdcall SetThreadExecutionState(
        _In_ EXECUTION_STATE esFlags);

namespace wb::base::win {

/**
 * @brief The thread's execution requirements.  This parameter can be one or
 * more of the following values.
 */
enum class ScopedThreadExecutionStateFlags : EXECUTION_STATE {
  /**
   * @brief Error occured.
   */
  kError = 0x0000'0000UL,
  /**
   * @brief Informs the system that the state being set should remain in effect
   * until the next call that uses kContinuous and one of the other state
   * flags is cleared.
   */
  kContinuous = 0x8000'0000UL,
  /**
   * @brief Forces the system to be in the working state by resetting the system
   * idle timer.
   */
  kSystemRequired = 0x0000'0001UL,
  /**
   * @brief Forces the display to be on by resetting the display idle timer.
   */
  kDisplayRequired = 0x0000'0002UL,
  /**
   * @brief This value is not supported.  If kUserPresent is combined with other
   * esFlags values, the call will fail and none of the specified states will be
   * set.
   */
  kUserPresent = 0x0000'0004UL,
  /**
   * @brief Away mode should be used only by media-recording and
   * media-distribution applications that must perform critical background
   * processing on desktop computers while the computer appears to be sleeping.
   */
  kAwayModeRequired = 0x0000'0040UL | kContinuous,
};

/**
 * @brief operator|
 * @param left Left.
 * @param right Right.
 * @return Left | Right.
 */
[[nodiscard]] constexpr ScopedThreadExecutionStateFlags operator|(
    ScopedThreadExecutionStateFlags left,
    ScopedThreadExecutionStateFlags right) noexcept {
  return static_cast<ScopedThreadExecutionStateFlags>(underlying_cast(left) |
                                                      underlying_cast(right));
}

/**
 * @brief operator&
 * @param left Left.
 * @param right Right.
 * @return Left & Right.
 */
[[nodiscard]] constexpr ScopedThreadExecutionStateFlags operator&(
    ScopedThreadExecutionStateFlags left,
    ScopedThreadExecutionStateFlags right) noexcept {
  return static_cast<ScopedThreadExecutionStateFlags>(underlying_cast(left) &
                                                      underlying_cast(right));
}

/**
 * @brief Enables an application to inform the system that it is in use, thereby
 * preventing the system from entering sleep or turning off the display while
 * the application is running.
 */
class ScopedThreadExecutionState {
 public:
  /**
   * @brief Create ScopedThreadExecutionState by flags.
   * @param flags ScopedThreadExecutionStateFlags.
   * @return ScopedThreadExecutionState.
   */
  static std2::result<ScopedThreadExecutionState> New(
      _In_ ScopedThreadExecutionStateFlags flags) noexcept {
    ScopedThreadExecutionState state{flags};
    return !state.error_code()
               ? std2::result<ScopedThreadExecutionState>{std::move(state)}
               : std2::result<ScopedThreadExecutionState>{state.error_code()};
  }

  ScopedThreadExecutionState(ScopedThreadExecutionState&& s) noexcept
      : old_flags_{std::move(s.old_flags_)},
        error_code_{std::move(s.error_code_)} {
    s.error_code_ = std2::posix_last_error_code(EINVAL);
  }
  ScopedThreadExecutionState& operator=(ScopedThreadExecutionState&&) = delete;

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(ScopedThreadExecutionState);

  ~ScopedThreadExecutionState() noexcept {
    if (!error_code()) {
      G3CHECK(!!::SetThreadExecutionState(base::underlying_cast(
          old_flags_ & ScopedThreadExecutionStateFlags::kContinuous)));
    }
  }

 private:
  /**
   * @brief Old thread execution state flags.
   */
  ScopedThreadExecutionStateFlags old_flags_;

  [[maybe_unused]] std::array<std::byte, sizeof(char*) - sizeof(old_flags)>;

  /**
   * @brief Thread execution state acquire error code.
   */
  std::error_code error_code_;

  /**
   * @brief Creates ScopedThreadExecutionState
   * @param flags ScopedThreadExecutionStateFlags
   * @return nothing.
   */
  explicit ScopedThreadExecutionState(
      _In_ ScopedThreadExecutionStateFlags flags) noexcept
      : old_flags_{static_cast<ScopedThreadExecutionStateFlags>(
            ::SetThreadExecutionState(underlying_cast(flags)))},
        error_code_{(old_flags_ != ScopedThreadExecutionStateFlags::kError
                         ? std2::ok_code
                         : std2::system_last_error_code())} {}

  /**
   * @brief Get initialization error code.
   * @return Error code.
   */
  [[nodiscard]] std::error_code error_code() const noexcept {
    return error_code_;
  }
};

}  // namespace wb::base::win

#endif  // !WB_BASE_WIN_SCOPED_THREAD_EXECUTION_STATE_H_
