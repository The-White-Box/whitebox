// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Provides an API for looking up localized message strings.

#include "scoped_process_locale.h"
//
#include "base/deps/googletest/gtest/gtest.h"

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(ScopedProcessLocaleTest, SetAllUtf8LocaleAsCurrentInScope) {
  using namespace wb::base::intl;

  // During program startup, the equivalent of std::setlocale(LC_ALL, "C"); is
  // executed before any user code is run.
  const char *out_of_scope_locale1{std::setlocale(LC_ALL, nullptr)};

  {
    const ScopedProcessLocale scoped_process_locale{
        ScopedProcessLocaleCategory::kAll, locales::kUtf8Locale};
    const auto maybe_current_locale = scoped_process_locale.GetCurrentLocale();

    ASSERT_TRUE(maybe_current_locale.has_value());

    const char *in_scope_locale{std::setlocale(LC_ALL, nullptr)};

    EXPECT_STREQ(in_scope_locale, maybe_current_locale.value().c_str());
    EXPECT_STRNE(out_of_scope_locale1, maybe_current_locale.value().c_str());
    EXPECT_TRUE(
        maybe_current_locale.value().find("utf8") != std::string::npos ||
        maybe_current_locale.value().find("UTF-8") != std::string::npos);
  }

  const char *out_of_scope_locale2{std::setlocale(LC_ALL, nullptr)};
  EXPECT_STREQ(out_of_scope_locale1, out_of_scope_locale2);
}
