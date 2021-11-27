// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// g3log death tests utils.

#include "g3log_death_utils_tests.h"

#include <csignal>
#include <cstddef>

namespace wb::base::tests_internal {

#ifdef WB_OS_WIN
[[nodiscard]] DeathTestResult<testing::ExitedWithCode>
#else
[[nodiscard]] DeathTestResult<testing::KilledBySignal>
#endif
MakeG3LogCheckFailureDeathTestResult(
    [[maybe_unused]] std::string message) noexcept {
#ifdef NDEBUG
#ifdef WB_OS_WIN
  // Windows handle SIGABRT and exit with code 3.  See
  // https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/raise?view=msvc-160#remarks
  constexpr int death_exit_code{3};
#else
  constexpr int death_signal_num{SIGABRT};
#endif

  std::string death_message{std::move(message)};
#else  // NDEBUG
#ifdef WB_OS_WIN
  // TODO(dimhotepus): Why STATUS_ACCESS_VIOLATION?
  constexpr unsigned long kStatusAccessViolation{0xC0000005L};
  constexpr int death_exit_code{static_cast<int>(kStatusAccessViolation)};
#else
  constexpr int death_signal_num{SIGTRAP};
#endif

  // In DEBUG mode message is not printed.
  std::string death_message;
#endif  // !NDEBUG

#ifdef WB_OS_WIN
  const auto exited_with_code = testing::ExitedWithCode{death_exit_code};
  return DeathTestResult<testing::ExitedWithCode>(exited_with_code,
                                                  std::move(death_message));
#else
  const auto killed_by_signal = testing::KilledBySignal{death_signal_num};
  return DeathTestResult<testing::KilledBySignal>(killed_by_signal,
                                                  std::move(death_message));
#endif
}

}  // namespace wb::base::tests_internal
