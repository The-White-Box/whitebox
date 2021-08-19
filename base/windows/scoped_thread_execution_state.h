// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Enables an application to inform the system that it is in use, thereby
// preventing the system from entering sleep or turning off the display while
// the application is running.

#ifndef WB_BASE_WINDOWS_SCOPED_THREAD_EXECUTION_STATE_H_
#define WB_BASE_WINDOWS_SCOPED_THREAD_EXECUTION_STATE_H_

#include <cstddef>  // std::byte

#include "base/base_macroses.h"
#include "base/deps/g3log/g3log.h"
#include "base/std_ext/system_error_ext.h"

using EXECUTION_STATE = unsigned long;

extern "C" WB_ATTRIBUTE_DLL_IMPORT
    EXECUTION_STATE __stdcall SetThreadExecutionState(
        _In_ EXECUTION_STATE esFlags);

namespace wb::base::windows {
/**
 * @brief The thread's execution requirements.  This parameter can be one or
 * more of the following values.
 */
enum class ScopedThreadExecutionStateFlags : EXECUTION_STATE {
  /**
   * @brief Error occured.
   */
  kError = 0x00000000UL,
  /**
   * @brief Informs the system that the state being set should remain in effect
   * until the next call that uses kContinuous and one of the other state
   * flags is cleared.
   */
  kContinuous = 0x80000000UL,
  /**
   * @brief Forces the system to be in the working state by resetting the system
   * idle timer.
   */
  kSystemRequired = 0x00000001UL,
  /**
   * @brief Forces the display to be on by resetting the display idle timer.
   */
  kDisplayRequired = 0x00000002UL,
  /**
   * @brief This value is not supported.  If kUserPresent is combined with other
   * esFlags values, the call will fail and none of the specified states will be
   * set.
   */
  kUserPresent = 0x00000004UL,
  /**
   * @brief Away mode should be used only by media-recording and
   * media-distribution applications that must perform critical background
   * processing on desktop computers while the computer appears to be sleeping.
   */
  kAwayModeRequired = 0x00000040UL | kContinuous,
};

/**
 * @brief operator|
 * @param left Left.
 * @param right Right.
 * @return Left | Right.
 */
[[nodiscard]] ScopedThreadExecutionStateFlags operator|(
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
[[nodiscard]] ScopedThreadExecutionStateFlags operator&(
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
  static std_ext::os_res<ScopedThreadExecutionState> New(
      _In_ ScopedThreadExecutionStateFlags flags) noexcept {
    ScopedThreadExecutionState state{flags};

    return !state.error_code()
               ? std_ext::os_res<ScopedThreadExecutionState>{std::move(state)}
               : std_ext::os_res<ScopedThreadExecutionState>{
                     state.error_code()};
  }

  ScopedThreadExecutionState(ScopedThreadExecutionState&& s) noexcept
      : old_flags_{std::move(s.old_flags_)},
        error_code_{std::move(s.error_code_)} {
    s.error_code_ = std::error_code(EINVAL, std::system_category());
  }
  ScopedThreadExecutionState& operator=(ScopedThreadExecutionState&&) = delete;

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(ScopedThreadExecutionState);

  ~ScopedThreadExecutionState() noexcept {
    if (!error_code()) {
      G3CHECK(!!::SetThreadExecutionState(underlying_cast(
          old_flags_ & ScopedThreadExecutionStateFlags::kContinuous)));
    }
  }

 private:
  /**
   * @brief Old thread execution state flags.
   */
  ScopedThreadExecutionStateFlags old_flags_;

  [[maybe_unused]] std::byte pad_[sizeof(char*) - sizeof(old_flags)];
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
                         ? std::error_code{}
                         : wb::base::std_ext::GetThreadErrorCode())} {}

  /**
   * @brief Get initialization error code.
   * @return Error code.
   */
  [[nodiscard]] std::error_code error_code() const noexcept {
    return error_code_;
  }
};
}  // namespace wb::base::windows

#endif  // !WB_BASE_WINDOWS_SCOPED_THREAD_EXECUTION_STATE_H_