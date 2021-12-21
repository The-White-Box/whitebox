// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// <string_view> extensions.

#include "string_view_ext.h"

#include "base/deps/googletest/gtest/gtest.h"
#include "base/tests/g3log_death_utils.h"

using namespace wb::base::std2;

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(StringViewExtTest, StartsWithHasEmptyString) {
  EXPECT_FALSE(starts_with("", '\0'));
  EXPECT_FALSE(starts_with("", ' '));
  EXPECT_FALSE(starts_with("", 'a'));

  EXPECT_FALSE(starts_with(std::string{}, '\0'));
  EXPECT_FALSE(starts_with(std::string{}, ' '));
  EXPECT_FALSE(starts_with(std::string{}, 'a'));
}

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(StringViewExtTest, StartsWithValue) {
  EXPECT_FALSE(starts_with("a", '\0'));
  EXPECT_FALSE(starts_with("abc", 'b'));
  EXPECT_FALSE(starts_with("abc", 'c'));
  EXPECT_TRUE(starts_with("abc", 'a'));

  EXPECT_FALSE(starts_with(std::string{"abc"}, '\0'));
  EXPECT_FALSE(starts_with(std::string{"abc"}, 'b'));
  EXPECT_FALSE(starts_with(std::string{"abc"}, 'c'));
  EXPECT_TRUE(starts_with(std::string{"abc"}, 'a'));
}

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(StringViewExtTest, EndsWithHasEmptyString) {
  EXPECT_FALSE(ends_with("", '\0'));
  EXPECT_FALSE(ends_with("", ' '));
  EXPECT_FALSE(ends_with("", 'a'));

  EXPECT_FALSE(ends_with(std::string{}, '\0'));
  EXPECT_FALSE(ends_with(std::string{}, ' '));
  EXPECT_FALSE(ends_with(std::string{}, 'a'));
}

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(StringViewExtTest, EndsWithHasValue) {
  EXPECT_FALSE(ends_with("a", '\0'));
  EXPECT_FALSE(ends_with("abc", 'b'));
  EXPECT_FALSE(ends_with("abc", 'a'));
  EXPECT_TRUE(ends_with("abc", 'c'));

  EXPECT_FALSE(ends_with(std::string{"abc"}, '\0'));
  EXPECT_FALSE(ends_with(std::string{"abc"}, 'b'));
  EXPECT_FALSE(ends_with(std::string{"abc"}, 'a'));
  EXPECT_TRUE(ends_with(std::string{"abc"}, 'c'));
}

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
#else  // WB_COMPILER_HAS_DEBUG
// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(StringViewExtTest, EndsWithStringWhenNullptr) {
  EXPECT_FALSE(ends_with(std::string{""}, nullptr));
  EXPECT_FALSE(ends_with(std::string{"a"}, nullptr));
  EXPECT_FALSE(ends_with(std::string{"abc"}, nullptr));
}
#endif  // !WB_COMPILER_HAS_DEBUG
#endif  // GTEST_HAS_DEATH_TEST
