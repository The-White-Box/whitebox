// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// g3log death tests utils.

#include "g3log_death_utils.h"

#include <csignal>
#include <cstddef>

namespace wb::base::tests_internal {

#ifdef WB_OS_WIN
[[nodiscard]] DeathTestResult<testing::ExitedWithCode>
#else
[[nodiscard]] DeathTestResult<testing::KilledBySignal>
#endif
MakeG3LogCheckFailureDeathTestResult([[maybe_unused]] std::string message
#ifdef WB_OS_WIN
                                     ,
                                     int exit_code
#endif
                                     ) noexcept {
#ifdef NDEBUG
  std::string death_message{std::move(message)};
#else   // NDEBUG
  // In DEBUG mode message is not printed.
  std::string death_message;
#endif  // !NDEBUG

#ifdef WB_OS_WIN
  const auto exited_with_code = testing::ExitedWithCode{exit_code};
  return DeathTestResult<testing::ExitedWithCode>(exited_with_code,
                                                  std::move(death_message));
#else
  constexpr int death_signal_num{SIGABRT};
  const auto killed_by_signal = testing::KilledBySignal{death_signal_num};

  return DeathTestResult<testing::KilledBySignal>(killed_by_signal,
                                                  std::move(death_message));
#endif
}

}  // namespace wb::base::tests_internal
