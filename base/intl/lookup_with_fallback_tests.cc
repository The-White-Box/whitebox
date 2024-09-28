// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Provides an API for looking up localized message strings.

#include "lookup_with_fallback.h"
//
#include "l18n.h"
//
#include "base/deps/googletest/gtest/gtest.h"

namespace {

/**
 * Computes hash for |string|.
 * @param string String to hash.
 * @return String hash.
 */
[[nodiscard]] WB_ATTRIBUTE_CONST WB_ATTRIBUTE_FORCEINLINE constexpr uint64_t
hash(std::string_view &&string) noexcept {
  return wb::base::intl::I18nStringViewHash{}(string);
}

}  // namespace

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(LookupWithFallbackTest, NewUnknownLocale) {
  using namespace wb::base::intl;

  const auto result = LookupWithFallback::New({"unknown-locale"});

  ASSERT_FALSE(result);
  EXPECT_EQ(Status::kArgumentError, result.error());
}

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(LookupWithFallbackTest, NewEnUsUtf8Locale) {
  using namespace wb::base::intl;

  const std::string kTestFallbackString{"Fallback"};

  const auto lookup =
      LookupWithFallback::New({"en_US.UTF-8"}, kTestFallbackString);

  ASSERT_TRUE(lookup);
  EXPECT_EQ(StringLayout::LeftToRight, lookup->Layout());

  {
    const auto localized =
        lookup->String(hash("Unable to create main '{0}' window."));

    EXPECT_EQ(std::string{"Unable to create main '{0}' window."}, localized);
  }

  {
    const auto non_localized = lookup->String(hash("Unknown string."));

    EXPECT_EQ(kTestFallbackString, non_localized);
  }

  {
    auto formatted = lookup->Format(hash("Unable to create main '{0}' window."),
                                    fmt::make_format_args("WhiteBox"));

    EXPECT_EQ(std::string{"Unable to create main 'WhiteBox' window."},
              formatted);
  }

  {
    auto formatted_too_many_args =
        lookup->Format(hash("Unable to create main '{0}' window."),
                       fmt::make_format_args("WhiteBox", "Addiitonal arg"));

    EXPECT_EQ(std::string{"Unable to create main 'WhiteBox' window."},
              formatted_too_many_args);
  }

  {
    auto non_formatted = lookup->Format(hash("Unknown string '{0}'."),
                                        fmt::make_format_args("WhiteBox"));

    EXPECT_EQ(kTestFallbackString, non_formatted);
  }

  {
    auto non_formatted_too_many_args =
        lookup->Format(hash("Unknown string '{0}'."),
                       fmt::make_format_args("WhiteBox", "Additional arg"));

    EXPECT_EQ(kTestFallbackString, non_formatted_too_many_args);
  }
}