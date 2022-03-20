// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Scoped the calling thread's last-error code value.

#include "scoped_thread_last_error.h"
//
#include <winerror.h>

#include "base/deps/googletest/gtest/gtest.h"
#include "build/compiler_config.h"

using namespace wb::base::win::error_handling;

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(ScopedThreadLastErrorTests, ShouldSetThreadLastErrorOutOfScopeTest) {
  const auto check_last_error_not_set = []() noexcept {
    const auto actual_error = ::GetLastError();

    EXPECT_EQ(actual_error, ERROR_SUCCESS);
  };

  check_last_error_not_set();

  {
    const ScopedThreadLastError scoped_error{ERROR_INVALID_FUNCTION};

    EXPECT_EQ(::GetLastError(), ERROR_SUCCESS);
  }

  EXPECT_EQ(::GetLastError(), ERROR_INVALID_FUNCTION);

  {
    ::SetLastError(ERROR_PATH_NOT_FOUND);

    const ScopedThreadLastError scoped_error;

    EXPECT_EQ(::GetLastError(), ERROR_PATH_NOT_FOUND);
  }

  EXPECT_EQ(::GetLastError(), ERROR_PATH_NOT_FOUND);

  ::SetLastError(ERROR_SUCCESS);
}
