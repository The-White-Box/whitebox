// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Provides an API for looking up localized message strings.

#include "l18n.h"
//
#include "base/deps/fmt/format.h"
#include "base/deps/googletest/gtest/gtest.h"
#include "base/tests/g3log_death_utils_tests.h"

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(L18nTest, CallOperatorHashes) {
  using namespace wb::base::intl;

  static_assert(I18nStringViewHash{}("") == 0);
  static_assert(I18nStringViewHash{}("", 1) == 0);
  static_assert(I18nStringViewHash{}("a", 0) == 194U);
  static_assert(I18nStringViewHash{}("a", 1) == 0);
  static_assert(I18nStringViewHash{}("1", 0) == 98U);
  static_assert(I18nStringViewHash{}(" ", 0) == 64U);
  static_assert(I18nStringViewHash{}("/", 0) == 94U);
  static_assert(I18nStringViewHash{}("abc", 0) == 1859U);
  static_assert(I18nStringViewHash{}("ABC", 0) == 739U);
  static_assert(I18nStringViewHash{}("123", 0) == 947U);
  static_assert(I18nStringViewHash{}("123", 1) == 977U);
  static_assert(I18nStringViewHash{}("123", 2) == 765U);
  static_assert(I18nStringViewHash{}("123", 3) == 0U);
  static_assert(I18nStringViewHash{}("ABC 123", 0) == 4973U);
  static_assert(I18nStringViewHash{}("ABC 123", 1) == 5103U);
}

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(L18nTest, l18nLookups) {
  using namespace wb::base::intl;

  const auto result = LookupWithFallback::New({"en_US.UTF-8"});
  const LookupWithFallback *lookup{std::get_if<LookupWithFallback>(&result)};

  ASSERT_NE(nullptr, lookup);

  EXPECT_EQ(l18n(*lookup, "Unknown string."), kFallbackString)
      << "Missed l18n should result in fallback string.";
  EXPECT_EQ(l18n(*lookup, "Boot Manager - Error"), "Boot Manager - Error");
}

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(L18nTest, l18nFmtLookups) {
  using namespace wb::base::intl;

  const auto result = LookupWithFallback::New({"en_US.UTF-8"});
  const LookupWithFallback *lookup{std::get_if<LookupWithFallback>(&result)};

  ASSERT_NE(nullptr, lookup);

  EXPECT_EQ(l18n_fmt(*lookup, "Unknown string."), kFallbackString)
      << "Missed l18n_fmt should result in fallback string.";
  EXPECT_EQ(l18n_fmt(*lookup, "Boot Manager - Error"), "Boot Manager - Error");
  EXPECT_EQ(l18n_fmt(*lookup, "Can't load boot manager '{0}'.", "bootmgr.so"),
            "Can't load boot manager 'bootmgr.so'.");
  EXPECT_EQ(l18n_fmt(*lookup, "Can't load boot manager '{0}'.", 1),
            "Can't load boot manager '1'.");
  EXPECT_EQ(
      l18n_fmt(*lookup, "Can't load boot manager '{0}'.", "bootmgr.so", "12"),
      "Can't load boot manager 'bootmgr.so'.")
      << "Should ignore additional args.";
}

#ifdef GTEST_HAS_DEATH_TEST
// NOLINTNEXTLINE(cert-err58-cpp, cppcoreguidelines-avoid-non-const-global-variables, cppcoreguidelines-owning-memory)
GTEST_TEST(L18nTestDeathTest, MissedArgumentTriggersTerminate) {
  using namespace wb::base;
  using namespace wb::base::intl;

  GTEST_FLAG_SET(death_test_style, "threadsafe");

  const auto result = LookupWithFallback::New({"en_US.UTF-8"});
  const LookupWithFallback *lookup{std::get_if<LookupWithFallback>(&result)};

  ASSERT_NE(nullptr, lookup);

  const auto triggerTerminate = [&]() {
    (void)l18n_fmt(*lookup, "Can't load boot manager '{0}'.");
  };

  const auto test_result = tests_internal::MakeG3LogCheckFailureDeathTestResult(
      "argument not found");

  EXPECT_EXIT(triggerTerminate(), test_result.exit_predicate,
              test_result.message);
}
#endif