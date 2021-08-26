// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// <cstring> extensions.

#include "cstring_ext.h"
//
#include "base/deps/googletest/gtest/gtest.h"

using namespace wb::base::std_ext;

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(CStringExtTests, BitwiseMemset) {
  unsigned int values[] = {1, 2, 3};

  const unsigned int(*actual)[3]{&BitwiseMemset(values, 0)};
  const unsigned int(*expected)[3]{&values};

  EXPECT_EQ(expected, actual);
  EXPECT_EQ((*actual)[0], 0U);
  EXPECT_EQ((*actual)[1], 0U);
  EXPECT_EQ((*actual)[2], 0U);
}

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(CStringExtTests, BitwiseCompare) {
  unsigned int values1[] = {1, 1, 3};
  unsigned int values2[] = {1, 2, 3};

  EXPECT_LT(BitwiseCompare(values1, values2), 0);

  values1[1] = 2;
  EXPECT_EQ(BitwiseCompare(values1, values2), 0);

  values1[1] = 3;
  EXPECT_GT(BitwiseCompare(values1, values2), 0);
}

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(CStringExtTests, BitwiseCopy) {
  unsigned int values1[] = {0, 0, 0};
  unsigned int values2[] = {1, 2, 3};

  const unsigned int(*actual)[3]{&BitwiseCopy(values1, values2)};
  const unsigned int(*expected)[3]{&values1};

  EXPECT_EQ(expected, actual);
  EXPECT_EQ((*actual)[0], 1U);
  EXPECT_EQ((*actual)[1], 2U);
  EXPECT_EQ((*actual)[2], 3U);
}
