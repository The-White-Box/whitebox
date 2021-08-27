// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.

#include "string_view_ext.h"
//
#ifdef WB_OS_WIN
#include "base/win/windows_light.h"
#else
#include <signal.h>  // SIGABRT
#endif

#include "base/deps/googletest/gtest/gtest.h"

using namespace wb::base::std_ext;

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(StringViewExtTests, StartsWithHasEmptyString) {
  EXPECT_FALSE(starts_with("", '\0'));
  EXPECT_FALSE(starts_with("", ' '));
  EXPECT_FALSE(starts_with("", 'a'));

  EXPECT_FALSE(starts_with(std::string{}, '\0'));
  EXPECT_FALSE(starts_with(std::string{}, ' '));
  EXPECT_FALSE(starts_with(std::string{}, 'a'));
}

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(StringViewExtTests, StartsWithValue) {
  EXPECT_FALSE(starts_with("a", '\0'));
  EXPECT_FALSE(starts_with("abc", 'b'));
  EXPECT_FALSE(starts_with("abc", 'c'));
  EXPECT_TRUE(starts_with("abc", 'a'));

  EXPECT_FALSE(starts_with(std::string{"abc"}, '\0'));
  EXPECT_FALSE(starts_with(std::string{"abc"}, 'b'));
  EXPECT_FALSE(starts_with(std::string{"abc"}, 'c'));
  EXPECT_TRUE(starts_with(std::string{"abc"}, 'a'));
}

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(StringViewExtTests, EndsWithHasEmptyString) {
  EXPECT_FALSE(ends_with("", '\0'));
  EXPECT_FALSE(ends_with("", ' '));
  EXPECT_FALSE(ends_with("", 'a'));

  EXPECT_FALSE(ends_with(std::string{}, '\0'));
  EXPECT_FALSE(ends_with(std::string{}, ' '));
  EXPECT_FALSE(ends_with(std::string{}, 'a'));
}

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(StringViewExtTests, EndsWithHasValue) {
  EXPECT_FALSE(ends_with("a", '\0'));
  EXPECT_FALSE(ends_with("abc", 'b'));
  EXPECT_FALSE(ends_with("abc", 'a'));
  EXPECT_TRUE(ends_with("abc", 'c'));

  EXPECT_FALSE(ends_with(std::string{"abc"}, '\0'));
  EXPECT_FALSE(ends_with(std::string{"abc"}, 'b'));
  EXPECT_FALSE(ends_with(std::string{"abc"}, 'a'));
  EXPECT_TRUE(ends_with(std::string{"abc"}, 'c'));
}

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(StringViewExtTests, EndsWithStringHasEmptyString) {
  EXPECT_TRUE(ends_with("", ""));
  EXPECT_FALSE(ends_with("", " "));
  EXPECT_FALSE(ends_with("", "a"));

  EXPECT_TRUE(ends_with(std::string{}, ""));
  EXPECT_FALSE(ends_with(std::string{}, " "));
  EXPECT_FALSE(ends_with(std::string{}, "a"));
}

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(StringViewExtTests, EndsWithStringHasValue) {
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

#if WB_COMPILER_HAS_DEBUG
// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(StringViewExtDeathTest, EndsWithStringWhenNullptr) {
  GTEST_FLAG_SET(death_test_style, "threadsafe");
#ifdef WB_OS_WIN
  EXPECT_EXIT((void)!!ends_with("", nullptr),
              testing::ExitedWithCode(STATUS_BREAKPOINT), "");
  EXPECT_EXIT((void)!!ends_with("a", nullptr),
              testing::ExitedWithCode(STATUS_BREAKPOINT), "");
#else
  [[maybe_unused]] volatile bool ensure_result_used;

  EXPECT_EXIT(ensure_result_used = ends_with("", nullptr),
              testing::KilledBySignal(SIGTRAP), "");
  EXPECT_EXIT(ensure_result_used = ends_with("a", nullptr),
              testing::KilledBySignal(SIGTRAP), "");
#endif
}
#endif
