// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Scoped handler when new operator fails to allocate memory.

#include "scoped_new_handler.h"
//
#include <array>
#include <limits>

#ifdef WB_OS_WIN
#include "base/win/windows_light.h"
#else
#include <signal.h>  // SIGABRT
#endif

#include "base/deps/g3log/g3log.h"
#include "base/deps/googletest/gtest/gtest.h"

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(ScopedNewHandlerTest, SetNewFailureHandlerInScope) {
  using namespace wb::base;

  EXPECT_NE(&DefaultNewFailureHandler, std::get_new_handler());

  {
    const ScopedNewHandler scoped_new_handler{DefaultNewFailureHandler};

    EXPECT_EQ(&DefaultNewFailureHandler, std::get_new_handler());
  }

  EXPECT_NE(&DefaultNewFailureHandler, std::get_new_handler());
}

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(ScopedNewHandlerDeathTest, OutOfMemoryTriggersNewFailureHandler) {
  GTEST_FLAG_SET(death_test_style, "threadsafe");

  const wb::base::ScopedNewHandler scoped_new_handler{
      wb::base::DefaultNewFailureHandler};

  const auto triggerOom = []() noexcept {
    constexpr size_t kOomAllocSize{std::numeric_limits<unsigned>::max() >> 2U};
    std::array<int *, 1024> memory{};

    for (auto &block : memory) {
      block = new int[kOomAllocSize];
      block[kOomAllocSize - 1] = 0;
    }
  };

#ifdef WB_OS_WIN
#ifdef NDEBUG
  // Windows handle SIGABRT and exit with code 3.  See
  // https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/raise?view=msvc-160#remarks
  constexpr int kExitCodeForSigAbrt{3};
  constexpr char kMessage[]{
      "Failed to allocate memory bytes via new.  Please, ensure you "
      "have enough RAM to run the app.  Stopping the app."};
#else
  // TODO(dimhotepus): Why STATUS_ACCESS_VIOLATION ?
  constexpr int kExitCodeForSigAbrt{static_cast<int>(STATUS_ACCESS_VIOLATION)};
  // In debug mode message is not printed.
  constexpr char kMessage[]{""};
#endif
#elif defined(WB_OS_POSIX)
  constexpr int kExitCodeForSigAbrt{SIGABRT};
  constexpr char kMessage[]{
      "Failed to allocate memory bytes via new.  Please, ensure you "
      "have enough RAM to run the app.  Stopping the app."};
#endif

  EXPECT_EXIT(triggerOom(), testing::ExitedWithCode(kExitCodeForSigAbrt),
              kMessage);
}
