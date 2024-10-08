// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Controls whether the system will handle the specified types of serious errors
// or whether the calling thread will handle them.

#include "scoped_thread_error_mode.h"
//
#include "base/deps/googletest/gtest/gtest.h"
#include "build/compiler_config.h"

using namespace wb::base::win::error_handling;

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(ScopedThreadErrorModeTests, ShouldSetThreadErrorModeInScopeTest) {
  const auto check_mode_not_set = []() noexcept {
    const auto actual_mode = ::GetThreadErrorMode();

    ASSERT_TRUE((actual_mode &
                 wb::base::underlying_cast(
                     ScopedThreadErrorModeFlags::kFailOnCriticalErrors)) == 0);
    ASSERT_TRUE((actual_mode &
                 wb::base::underlying_cast(
                     ScopedThreadErrorModeFlags::kNoOpenFileErrorBox)) == 0);
  };

  check_mode_not_set();

  {
    auto mode_result = ScopedThreadErrorMode::New(
        ScopedThreadErrorModeFlags::kFailOnCriticalErrors |
        ScopedThreadErrorModeFlags::kNoOpenFileErrorBox);

    ASSERT_TRUE(mode_result.has_value());

    const auto actual_mode = ::GetThreadErrorMode();
    EXPECT_TRUE((actual_mode &
                 wb::base::underlying_cast(
                     ScopedThreadErrorModeFlags::kFailOnCriticalErrors)) != 0);
    EXPECT_TRUE((actual_mode &
                 wb::base::underlying_cast(
                     ScopedThreadErrorModeFlags::kNoOpenFileErrorBox)) != 0);
  }

  check_mode_not_set();
}

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(ScopedThreadErrorModeTests, ShouldMoveThreadErrorModeScopeTest) {
  const auto check_mode_not_set = []() noexcept {
    const auto actual_mode = ::GetThreadErrorMode();

    ASSERT_TRUE((actual_mode &
                 wb::base::underlying_cast(
                     ScopedThreadErrorModeFlags::kFailOnCriticalErrors)) == 0);
    ASSERT_TRUE((actual_mode &
                 wb::base::underlying_cast(
                     ScopedThreadErrorModeFlags::kNoOpenFileErrorBox)) == 0);
  };

  check_mode_not_set();

  {
    auto mode_result = ScopedThreadErrorMode::New(
        ScopedThreadErrorModeFlags::kFailOnCriticalErrors |
        ScopedThreadErrorModeFlags::kNoOpenFileErrorBox);

    ASSERT_TRUE(mode_result.has_value());

    {
      ScopedThreadErrorMode moved_mode = std::move(*mode_result);

      const auto actual_mode = ::GetThreadErrorMode();

      EXPECT_TRUE((actual_mode &
                   wb::base::underlying_cast(
                       ScopedThreadErrorModeFlags::kFailOnCriticalErrors)) !=
                  0);
      EXPECT_TRUE((actual_mode &
                   wb::base::underlying_cast(
                       ScopedThreadErrorModeFlags::kNoOpenFileErrorBox)) != 0);
    }

    check_mode_not_set();
  }

  check_mode_not_set();
}
