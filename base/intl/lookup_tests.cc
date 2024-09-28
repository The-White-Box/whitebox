// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Provides an API for looking up localized message strings.

#include "lookup.h"
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
GTEST_TEST(LookupTest, NewUnknownLocale) {
  using namespace wb::base::intl;

  EXPECT_EQ(Status::kArgumentError,
            Lookup::New({"unknown-locale"}).error_or(Status::kOk));
}

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(LookupTest, NewEnUsUtf8Locale) {
  using namespace wb::base::intl;

  const auto lookup = Lookup::New({"en_US.UTF-8"});

  ASSERT_TRUE(lookup);
  EXPECT_EQ(StringLayout::LeftToRight, lookup->Layout());

  {
    auto localized =
        lookup->String(hash("Unable to create main '{0}' window."));

    ASSERT_TRUE(localized);
    EXPECT_EQ(std::string{"Unable to create main '{0}' window."}, localized->get());
  }

  {
    auto non_localized = lookup->String(hash("Unknown string."));

    ASSERT_FALSE(non_localized);
    EXPECT_EQ(Status::kUnavailable, non_localized.error());
  }

  {
    auto formatted = lookup->Format(hash("Unable to create main '{0}' window."),
                                    fmt::make_format_args("WhiteBox"));

    ASSERT_TRUE(formatted);
    EXPECT_EQ(std::string{"Unable to create main 'WhiteBox' window."},
              *formatted);
  }

  {
    auto formatted_too_many_args =
        lookup->Format(hash("Unable to create main '{0}' window."),
                       fmt::make_format_args("WhiteBox", "Additional arg"));

    ASSERT_TRUE(formatted_too_many_args);
    EXPECT_EQ(std::string{"Unable to create main 'WhiteBox' window."},
              *formatted_too_many_args);
  }

  {
    auto non_formatted = lookup->Format(hash("Unknown string '{0}'."),
                                        fmt::make_format_args("WhiteBox"));

    ASSERT_FALSE(non_formatted);
    EXPECT_EQ(Status::kUnavailable, non_formatted.error());
  }

  {
    auto non_formatted_too_many_args =
        lookup->Format(hash("Unknown string '{0}'."),
                       fmt::make_format_args("WhiteBox", "Additional arg"));

    ASSERT_FALSE(non_formatted_too_many_args);
    EXPECT_EQ(Status::kUnavailable, non_formatted_too_many_args.error());
  }
}