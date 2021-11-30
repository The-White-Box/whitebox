// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// g3log death tests utils.

#ifndef WB_BASE_TESTS_G3LOG_DEATH_UTILS_TESTS_H_
#define WB_BASE_TESTS_G3LOG_DEATH_UTILS_TESTS_H_

#include <cstddef>
#include <string>
#include <type_traits>

#include "base/macroses.h"
#include "base/deps/googletest/gtest/gtest.h"
#include "build/build_config.h"

namespace wb::base::tests_internal {

/**
 * @brief Death test exit predicate concept.
 * @tparam TPredicate Type to be death test exit predicate.
 */
template <typename TPredicate>
using death_test_exit_predicate_concept =
    std::enable_if_t<std::is_same_v<TPredicate, testing::ExitedWithCode>
#ifndef WB_OS_WIN
                     || std::is_same_v<TPredicate, testing::KilledBySignal>
#endif
                     >;

/**
 * @brief Death test result.
 * @tparam TPredicate Death test predicate.
 */
template <typename TPredicate,
          typename = death_test_exit_predicate_concept<TPredicate>>
struct DeathTestResult {
  /**
   * @brief Creates death test result.
   * @param exit_predicate_ Exit predicate.
   * @param message_ Exit message.
   */
  DeathTestResult(TPredicate exit_predicate_, std::string message_) noexcept
      : exit_predicate{exit_predicate_}, message{std::move(message_)} {}

  /**
   * @brief Exit predicate.
   */
  TPredicate exit_predicate;
  WB_ATTRIBUTE_UNUSED_FIELD std::byte pad_[4] = {};
  /**
   * @brief Exit message.
   */
  std::string message;

  DeathTestResult(DeathTestResult &) = default;
  DeathTestResult &operator=(DeathTestResult &) = delete;
  DeathTestResult(DeathTestResult &&) = default;
  DeathTestResult &operator=(DeathTestResult &&) = delete;
};

#ifdef WB_OS_WIN
/**
 * @brief Creates death test result for g3log {D}CHECK failure.
 * @param message Exit message.
 * @returns Death test result.
 */
[[nodiscard]] DeathTestResult<testing::ExitedWithCode>
MakeG3LogCheckFailureDeathTestResult(
    [[maybe_unused]] std::string message) noexcept;
#else
/**
 * @brief Creates death test result for g3log {D}CHECK failure.
 * @param message Exit message.
 * @returns Death test result.
 */
[[nodiscard]] DeathTestResult<testing::KilledBySignal>
MakeG3LogCheckFailureDeathTestResult(
    [[maybe_unused]] std::string message) noexcept;
#endif

}  // namespace wb::base::tests_internal

#endif  // !WB_BASE_TESTS_G3LOG_DEATH_UTILS_TESTS_H_
