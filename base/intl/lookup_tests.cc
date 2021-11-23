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
hash(std::string_view&& string) noexcept {
  return wb::base::intl::I18nStringViewHash{}(string);
}

}

// NOLINTNEXTLINE(cert-err58-cpp, cppcoreguidelines-avoid-non-const-global-variables, cppcoreguidelines-owning-memory)
GTEST_TEST(LookupTest, NewUnknownLocale) {
  using namespace wb::base::intl;
  
  const auto result = Lookup::New({"unknown-locale"});
  const Status *status{std::get_if<Status>(&result)};
  
  ASSERT_NE(nullptr, status);
  EXPECT_EQ(Status::kArgumentError, *status);
}

// NOLINTNEXTLINE(cert-err58-cpp, cppcoreguidelines-avoid-non-const-global-variables, cppcoreguidelines-owning-memory)
GTEST_TEST(LookupTest, NewEnUsUtf8Locale) {
  using namespace wb::base::intl;
  
  const auto result = Lookup::New({"en_US.UTF-8"});
  const Lookup *lookup{std::get_if<Lookup>(&result)};
  
  ASSERT_NE(nullptr, lookup);
  EXPECT_EQ(StringLayout::LeftToRight, lookup->Layout());
  
  {
    auto localized = lookup->String(hash("Unable to create main '{0}' window."));
    const auto *string_ptr{std::get_if<Lookup::Ref<const std::string>>(&localized)};
    
    ASSERT_NE(nullptr, string_ptr);
    
    const std::string &string{*string_ptr};
    EXPECT_EQ(std::string{"Unable to create main '{0}' window."}, string);
  }
  
  {
    auto non_localized = lookup->String(hash("Unknown string."));
    const Status *status{std::get_if<Status>(&non_localized)};
  
    ASSERT_NE(nullptr, status);
    EXPECT_EQ(Status::kUnavailable, *status);
  }

  {
    auto formatted = lookup->Format(hash("Unable to create main '{0}' window."),
                                    fmt::make_format_args("WhiteBox"));
    const auto *formatted_ptr{std::get_if<std::string>(&formatted)};

    ASSERT_NE(nullptr, formatted_ptr);
    EXPECT_EQ(std::string{"Unable to create main 'WhiteBox' window."},
              *formatted_ptr);
  }
  
  {
    auto non_formatted = lookup->Format(hash("Unknown string '{0}'."),
                                        fmt::make_format_args("WhiteBox"));
    const Status *status{std::get_if<Status>(&non_formatted)};

    ASSERT_NE(nullptr, status);
    EXPECT_EQ(Status::kUnavailable, *status);
  }
}