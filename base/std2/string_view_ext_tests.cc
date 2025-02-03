// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// <string_view> extensions.

#include "base/deps/googletest/gtest/gtest.h"
#include "base/tests/g3log_death_utils.h"
#include "string_view_ext.h"

using namespace wb::base::std2;

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(StringViewExtTest, EndsWithStringHasEmptyString) {
  EXPECT_TRUE(ends_with("", ""));
  EXPECT_FALSE(ends_with("", " "));
  EXPECT_FALSE(ends_with("", "a"));

  EXPECT_TRUE(ends_with(std::string{}, ""));
  EXPECT_FALSE(ends_with(std::string{}, " "));
  EXPECT_FALSE(ends_with(std::string{}, "a"));
}

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(StringViewExtTest, EndsWithStringHasValue) {
  EXPECT_TRUE(ends_with("a", ""));
  EXPECT_FALSE(ends_with("abc", "b"));
  EXPECT_FALSE(ends_with("abc", "ab"));
  EXPECT_TRUE(ends_with("abc", "c"));
  EXPECT_TRUE(ends_with("abc", "bc"));
  EXPECT_TRUE(ends_with("abc", "abc"));

  EXPECT_TRUE(ends_with(std::string{"a"}, ""));
  EXPECT_FALSE(ends_with(std::string{"abc"}, "b"));
  EXPECT_FALSE(ends_with(std::string{"abc"}, "ab"));
  EXPECT_TRUE(ends_with(std::string{"abc"}, "c"));
  EXPECT_TRUE(ends_with(std::string{"abc"}, "bc"));
  EXPECT_TRUE(ends_with(std::string{"abc"}, "abc"));
}

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(StringViewExtTest, TrimSpaces) {
  char out[16];
  EXPECT_TRUE(TrimSpaces("", out));
  EXPECT_EQ(out[0], '\0');

  EXPECT_TRUE(TrimSpaces(" ", out));
  EXPECT_EQ(out[0], '\0');

  EXPECT_TRUE(TrimSpaces("  ", out));
  EXPECT_EQ(out[0], '\0');

  EXPECT_TRUE(TrimSpaces("   ", out));
  EXPECT_EQ(out[0], '\0');

  EXPECT_FALSE(TrimSpaces("abc", nullptr, 0));

  EXPECT_FALSE(TrimSpaces("a", out));
  EXPECT_STREQ("a", out);

  EXPECT_FALSE(TrimSpaces("abc", out));
  EXPECT_STREQ("abc", out);

  EXPECT_FALSE(TrimSpaces("a1 b2 c3", out));
  EXPECT_STREQ("a1 b2 c3", out);

  EXPECT_TRUE(TrimSpaces(" 123", out));
  EXPECT_STREQ("123", out);

  EXPECT_TRUE(TrimSpaces("  d a b", out));
  EXPECT_STREQ("d a b", out);

  EXPECT_TRUE(TrimSpaces("mnTb ", out));
  EXPECT_STREQ("mnTb", out);

  EXPECT_TRUE(TrimSpaces("m1 n2 T3 dd  ", out));
  EXPECT_STREQ("m1 n2 T3 dd", out);

  EXPECT_TRUE(TrimSpaces("m1 n2 T3 dd  ", out));
  EXPECT_STREQ("m1 n2 T3 dd", out);
}

#if GTEST_HAS_DEATH_TEST
#if WB_COMPILER_HAS_DEBUG
// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(StringViewExtDeathTest, EndsWithStringWhenNullptr) {
  GTEST_FLAG_SET(death_test_style, "threadsafe");

  const auto test_result =
      wb::base::tests_internal::MakeG3LogCheckFailureDeathTestResult("");

  EXPECT_EXIT([[maybe_unused]] const auto volatile v1 = ends_with("", nullptr),
              test_result.exit_predicate, test_result.message);
  EXPECT_EXIT([[maybe_unused]] const auto volatile v2 = ends_with("a", nullptr),
              test_result.exit_predicate, test_result.message);
  EXPECT_EXIT(
      [[maybe_unused]] const auto volatile v3 = ends_with("abc", nullptr),
      test_result.exit_predicate, test_result.message);
}
#else   // WB_COMPILER_HAS_DEBUG
// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(StringViewExtTest, EndsWithStringWhenNullptr) {
  EXPECT_FALSE(ends_with(std::string{""}, nullptr));
  EXPECT_FALSE(ends_with(std::string{"a"}, nullptr));
  EXPECT_FALSE(ends_with(std::string{"abc"}, nullptr));
}
#endif  // !WB_COMPILER_HAS_DEBUG
#endif  // GTEST_HAS_DEATH_TEST
