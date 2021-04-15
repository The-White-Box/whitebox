// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Scoped handler when new operator fails to allocate memory.

#include "scoped_new_handler.h"

#include "base/default_new_handler.h"
#include "base/deps/googletest/gtest/gtest.h"

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(ScopedNewHandlerTest, SetNewFailureHandlerInScope) {
  using namespace wb::base;

  EXPECT_NE(&DefaultNewFailureHandler, std::get_new_handler());

  {
    const ScopedNewHandler scoped_new_handler{DefaultNewFailureHandler, 0U};

    EXPECT_EQ(&DefaultNewFailureHandler, std::get_new_handler());
  }

  EXPECT_NE(&DefaultNewFailureHandler, std::get_new_handler());
}
