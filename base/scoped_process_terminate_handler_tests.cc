// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Scoped handler - process termination routine to be called by terminate.

#include "scoped_process_terminate_handler.h"
//
#include "base/deps/googletest/gtest/gtest.h"

#ifdef WB_OS_WIN
#include "base/win/windows_light.h"
#endif

// NOLINTNEXTLINE(cert-err58-cpp)
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

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(ScopedProcessTerminateHandlerDeathTest,
           TerminateTriggersTerminateHandler) {
  using namespace wb::base;

  GTEST_FLAG_SET(death_test_style, "threadsafe");

  const ScopedProcessTerminateHandler scoped_process_terminate_handler{
      DefaultProcessTerminateHandler};

  const auto triggerTerminate = []() { std::terminate(); };

#ifdef WB_OS_WIN
#ifdef NDEBUG
  // Windows handle SIGABRT and exit with code 3.  See
  // https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/raise?view=msvc-160#remarks
  constexpr int kExitCodeForSigAbrt{3};
  constexpr char kMessage[]{"Terminate called.  Stopping the app."};
#else
  // TODO(dimhotepus): Why STATUS_ACCESS_VIOLATION?
  constexpr int kExitCodeForSigAbrt{static_cast<int>(STATUS_ACCESS_VIOLATION)};
  // In debug mode message is not printed.
  // NOLINTNEXTLINE(modernize-avoid-c-arrays)
  constexpr char kMessage[]{""};
#endif
#else
#ifdef NDEBUG
  // Windows handle SIGABRT and exit with code 3.  See
  // https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/raise?view=msvc-160#remarks
  constexpr int kExitCodeForSigAbrt{3};
  // NOLINTNEXTLINE(modernize-avoid-c-arrays)
  constexpr char kMessage[]{"Terminate called.  Stopping the app."};
#else
  constexpr int kExitCodeForSigAbrt{SIGTRAP};
  // In debug mode message is not printed.
  const std::string kMessage;
#endif
#endif

#ifdef WB_OS_WIN
  EXPECT_EXIT(triggerTerminate(), testing::ExitedWithCode(kExitCodeForSigAbrt),
              kMessage);
#else
  EXPECT_EXIT(triggerTerminate(), testing::KilledBySignal(kExitCodeForSigAbrt),
              kMessage);
#endif
}
