// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Controls whether the system will handle the specified types of serious errors
// or whether the calling thread will handle them.

#ifndef WB_BASE_WIN_ERROR_HANDLING_SCOPED_THREAD_ERROR_MODE_H_
#define WB_BASE_WIN_ERROR_HANDLING_SCOPED_THREAD_ERROR_MODE_H_

#include <array>
#include <cstddef>  // std::byte

#include "base/deps/g3log/g3log.h"
#include "base/macroses.h"
#include "base/win/system_error_ext.h"
#include "build/compiler_config.h"

extern "C" WB_ATTRIBUTE_DLL_IMPORT unsigned long __stdcall GetThreadErrorMode();
extern "C" WB_ATTRIBUTE_DLL_IMPORT int __stdcall SetThreadErrorMode(
    _In_ unsigned long dwNewMode, _In_opt_ unsigned long* lpOldMode);

namespace wb::base::win::error_handling {

/**
 * @brief The thread error mode flags.
 */
enum class ScopedThreadErrorModeFlags : unsigned long {
  /**
   * @brief Use the system default, which is to display all error dialog boxes.
   */
  kNone = 0UL,
  /**
   * @brief The system does not display the critical-error-handler message box.
   * Instead, the system sends the error to the calling thread.
   */
  kFailOnCriticalErrors = 0x0001UL,
  /**
   * @brief The system does not display the Windows Error Reporting dialog.
   */
  kNoGpFaultErrorBox = 0x0002UL,
  /**
   * @brief The OpenFile function does not display a message box when it fails
   * to find a file.  Instead, the error is returned to the caller.  This error
   * mode overrides the OF_PROMPT flag.
   */
  kNoOpenFileErrorBox = 0x8000UL,
};

/**
 * @brief operator| for ScopedThreadErrorModeFlags
 * @param left Left.
 * @param right Right.
 * @return Left | Right.
 */
[[nodiscard]] constexpr ScopedThreadErrorModeFlags operator|(
    ScopedThreadErrorModeFlags left,
    ScopedThreadErrorModeFlags right) noexcept {
  return static_cast<ScopedThreadErrorModeFlags>(underlying_cast(left) |
                                                 underlying_cast(right));
}

/**
 * @brief Scoped thread error mode.
 */
class ScopedThreadErrorMode {
 public:
  /**
   * @brief Create new thread error mode.
   * @param error_mode_flags Error mode flags.
   * @return ScopedThreadErrorMode
   */
  static std2::result<ScopedThreadErrorMode> New(
      const ScopedThreadErrorModeFlags error_mode_flags) noexcept {
    auto mode = ScopedThreadErrorMode{error_mode_flags};
    return !mode.error_code()
               ? std2::result<ScopedThreadErrorMode>{std::move(mode)}
               : std2::result<ScopedThreadErrorMode>{std::unexpect,
                                                     mode.error_code()};
  }

  ScopedThreadErrorMode(ScopedThreadErrorMode&& m) noexcept
      : old_error_mode_{m.old_error_mode_}, error_code_{m.error_code_} {
    // Ensure no deinitialization occurs.
    m.error_code_ = std::error_code{EINVAL, std::generic_category()};
  }

  ScopedThreadErrorMode& operator=(ScopedThreadErrorMode&&) = delete;

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(ScopedThreadErrorMode);

  /**
   * @brief Restore previous thread error mode.
   */
  ~ScopedThreadErrorMode() noexcept {
    if (!error_code()) {
      G3CHECK(!!::SetThreadErrorMode(old_error_mode_, nullptr));
    }
  }

 private:
  /**
   * @brief Old thread error mode.
   */
  unsigned long old_error_mode_;

  [[maybe_unused]] std::array<std::byte,
                              sizeof(char*) - sizeof(old_error_mode_)>
      pad_;

  /**
   * @brief Set thread error mode error code.
   */
  std::error_code error_code_;

  /**
   * @brief Set thread error mode per scope.
   * @param error_mode_flags Thread error mode flags.
   * @return nothing.
   */
  explicit ScopedThreadErrorMode(
      const ScopedThreadErrorModeFlags error_mode_flags) noexcept
      : old_error_mode_{::GetThreadErrorMode()},
        error_code_{get_error(::SetThreadErrorMode(
            old_error_mode_ | underlying_cast(error_mode_flags), nullptr))} {
    G3DCHECK(!error_code());
  }

  /**
   * @brief Get error code.
   * @return Error code.
   */
  [[nodiscard]] std::error_code error_code() const noexcept {
    return error_code_;
  }
};

}  // namespace wb::base::win::error_handling

#endif  // !WB_BASE_WIN_ERROR_HANDLING_SCOPED_THREAD_ERROR_MODE_H_
