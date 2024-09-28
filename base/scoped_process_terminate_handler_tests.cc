// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Scoped handler - process termination routine to be called by terminate.

#include "scoped_process_terminate_handler.h"
//
#include "base/deps/googletest/gtest/gtest.h"
#include "base/tests/g3log_death_utils.h"

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(ScopedProcessTerminateHandlerTest, SetTerminateHandlerInScope) {
  using namespace wb::base;

  EXPECT_NE(&DefaultProcessTerminateHandler, std::get_terminate());

  {
    const ScopedProcessTerminateHandler scoped_process_terminate_handler{
        DefaultProcessTerminateHandler};

    EXPECT_EQ(&DefaultProcessTerminateHandler, std::get_terminate());
  }

  EXPECT_NE(&DefaultProcessTerminateHandler, std::get_terminate());
}

#ifdef GTEST_HAS_DEATH_TEST
// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(ScopedProcessTerminateHandlerDeathTest,
           TerminateTriggersTerminateHandler) {
  using namespace wb::base;

  GTEST_FLAG_SET(death_test_style, "threadsafe");

  const ScopedProcessTerminateHandler scoped_process_terminate_handler{
      DefaultProcessTerminateHandler};

  const auto test_result = tests_internal::MakeG3LogCheckFailureDeathTestResult(
      "Terminate called.  Stopping the app.");
  const auto triggerTerminate = []() { std::terminate(); };

  ASSERT_EXIT(triggerTerminate(), test_result.exit_predicate,
              test_result.message);
}
#endif