// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// <string> extensions.

#include "string_ext.h"
//
#include "base/deps/googletest/gtest/gtest.h"

using namespace wb::base::std2;

#ifdef WB_OS_WIN
// NOLINTNEXTLINE(cert-err58-cpp, cppcoreguidelines-avoid-non-const-global-variables, cppcoreguidelines-owning-memory)
GTEST_TEST(StringExtTests, WideToUTF8AndBack) {
  const std::wstring expected{L"Привет!"};

  EXPECT_EQ(expected, UTF8ToWide(WideToUTF8(expected)));
}

// NOLINTNEXTLINE(cert-err58-cpp, cppcoreguidelines-avoid-non-const-global-variables, cppcoreguidelines-owning-memory)
GTEST_TEST(StringExtTests, UTF8ToWideAndBack) {
  const std::string expected{"Hello!"};

  EXPECT_EQ(expected, WideToUTF8(UTF8ToWide(expected)));
}
#endif