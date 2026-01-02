// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Scoped handler when CRT detects an invalid argument.

#include "scoped_thread_invalid_parameter_handler.h"
//
#include <cstdio>

#include "base/deps/googletest/gtest/gtest.h"
#include "base/tests/g3log_death_utils.h"
#include "build/compiler_config.h"

using namespace wb::base::win::error_handling;

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(ScopedThreadInvalidParameterHandlerTests,
           ShouldSetPureCallHandlerInScopeTest) {
  ASSERT_NE(::_get_thread_local_invalid_parameter_handler(),
            DefaultThreadInvalidParameterHandler);

  {
    ScopedThreadInvalidParameterHandler scoped_thread_invalid_parameter_handler{
        DefaultThreadInvalidParameterHandler};

    EXPECT_EQ(::_get_thread_local_invalid_parameter_handler(),
              DefaultThreadInvalidParameterHandler);
  }

  EXPECT_NE(::_get_thread_local_invalid_parameter_handler(),
            DefaultThreadInvalidParameterHandler);
}

#ifdef GTEST_HAS_DEATH_TEST
// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(ScopedThreadInvalidParameterHandlerDeathTest,
           ShouldTerminateOnPureCallTest) {
  GTEST_FLAG_SET(death_test_style, "threadsafe");

  const auto triggerInvalidParameterCall = []() {
    ScopedThreadInvalidParameterHandler scoped_thread_invalid_parameter_handler{
        DefaultThreadInvalidParameterHandler};

    (void)tmpfile_s(nullptr);
  };

  const auto test_result =
      wb::base::tests_internal::MakeG3LogCheckFailureDeathTestResult(
          "Invalid parameter detected in function \\(null\\)\\.");

  WB_GCC_BEGIN_WARNING_OVERRIDE_SCOPE()
    WB_GCC_DISABLE_SWITCH_DEFAULT_WARNING()
    EXPECT_EXIT(triggerInvalidParameterCall(), test_result.exit_predicate,
                test_result.message);
  WB_GCC_END_WARNING_OVERRIDE_SCOPE()
}
#endif  // GTEST_HAS_DEATH_TEST