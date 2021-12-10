// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Provides an API for looking up localized message strings.

#include "scoped_process_locale.h"
//
#include "base/deps/googletest/gtest/gtest.h"

namespace {

/**
 * @brief Get current locale.
 * @return Current locale identifier.
 */
[[nodiscard]] std::string GetCurrentLocale() {
  const char *locale{std::setlocale(LC_ALL, nullptr)};
  return locale ? locale : "N/A";
}

}  // namespace

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(ScopedProcessLocaleTest, SetAllUtf8LocaleAsCurrentInScope) {
  using namespace wb::base::intl;

  // During program startup, the equivalent of std::setlocale(LC_ALL, "C"); is
  // executed before any user code is run.
  const std::string out_of_scope_locale1{GetCurrentLocale()};

  {
    const ScopedProcessLocale scoped_process_locale{
        ScopedProcessLocaleCategory::kAll, locales::kUtf8Locale};
    const auto maybe_current_locale = scoped_process_locale.GetCurrentLocale();
    if (maybe_current_locale.has_value()) {
      const std::string in_scope_locale{GetCurrentLocale()};

      EXPECT_EQ(in_scope_locale, maybe_current_locale.value().c_str());
      EXPECT_NE(out_of_scope_locale1, maybe_current_locale.value().c_str());
      EXPECT_TRUE(
          maybe_current_locale.value().find("utf8") != std::string::npos ||
          maybe_current_locale.value().find("UTF-8") != std::string::npos);
    }

    ASSERT_TRUE(maybe_current_locale.has_value());
  }

  const std::string out_of_scope_locale2{GetCurrentLocale()};
  EXPECT_EQ(out_of_scope_locale1, out_of_scope_locale2);
}
