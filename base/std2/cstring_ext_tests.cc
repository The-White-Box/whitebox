// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// <cstring> extensions.

#include "cstring_ext.h"
//
#include "base/deps/googletest/gtest/gtest.h"

using namespace wb::base::std2;

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(CStringExtTest, BitwiseMemset) {
  // NOLINTNEXTLINE(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
  unsigned int values[] = {1, 2, 3};

  // NOLINTNEXTLINE(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
  const unsigned int(*actual)[3]{&BitwiseMemset(values, 0)};
  // NOLINTNEXTLINE(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
  const unsigned int(*expected)[3]{&values};

  EXPECT_EQ(expected, actual);
  EXPECT_EQ((*actual)[0], 0U);
  EXPECT_EQ((*actual)[1], 0U);
  EXPECT_EQ((*actual)[2], 0U);
}

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(CStringExtTest, BitwiseCompare) {
  // NOLINTNEXTLINE(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
  unsigned int values1[] = {1, 1, 3};
  // NOLINTNEXTLINE(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
  unsigned int values2[] = {1, 2, 3};

  EXPECT_LT(BitwiseCompare(values1, values2), 0);

  values1[1] = 2;
  EXPECT_EQ(BitwiseCompare(values1, values2), 0);

  values1[1] = 3;
  EXPECT_GT(BitwiseCompare(values1, values2), 0);
}

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(CStringExtTest, BitwiseCopy) {
  // NOLINTNEXTLINE(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
  unsigned int values1[] = {0, 0, 0};
  // NOLINTNEXTLINE(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
  unsigned int values2[] = {1, 2, 3};

  // NOLINTNEXTLINE(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
  const unsigned int(*actual)[3]{&BitwiseCopy(values1, values2)};
  // NOLINTNEXTLINE(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
  const unsigned int(*expected)[3]{&values1};

  EXPECT_EQ(expected, actual);
  EXPECT_EQ((*actual)[0], 1U);
  EXPECT_EQ((*actual)[1], 2U);
  EXPECT_EQ((*actual)[2], 3U);
}
