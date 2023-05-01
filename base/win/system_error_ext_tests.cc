// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// <system_error> extensions for Windows.

#include "system_error_ext.h"
//
#include <winerror.h>
//
#include "base/deps/googletest/gtest/gtest.h"

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(SystemErrorExtWinTest, is_succeeded) {
  EXPECT_TRUE(wb::base::win::is_succeeded(S_OK));
  EXPECT_TRUE(wb::base::win::is_succeeded(S_FALSE));
  EXPECT_FALSE(wb::base::win::is_succeeded(E_FAIL));
}

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(SystemErrorExtWinTest, is_failed) {
  EXPECT_FALSE(wb::base::win::is_failed(S_OK));
  EXPECT_FALSE(wb::base::win::is_failed(S_FALSE));
  EXPECT_TRUE(wb::base::win::is_failed(E_FAIL));
}