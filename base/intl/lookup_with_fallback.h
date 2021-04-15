// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Provides an API for looking up localized message strings.
//
// Usage example:
//
// const std::string &hello_message{lookup_with_fallback.String(122)};
//
// or
//
// const std::string greetings{lookup_with_fallback.Format(123, user_name)};
//
// Based on
// https://cs.opensource.google/fuchsia/fuchsia/+/main:src/lib/intl/lookup/cpp/lookup.h

#ifndef WB_BASE_INTL_LOOKUP_WITH_FALLBACK_H_
#define WB_BASE_INTL_LOOKUP_WITH_FALLBACK_H_

#include "base/intl/lookup.h"

namespace wb::base::intl {

/**
 * @brief Fallback string.
 */
constexpr char kFallbackString[]{"N/A"};

/**
 * @brief The API used to look up localized messages by their unique message ID.
 */
class WB_BASE_API LookupWithFallback {
 public:
  LookupWithFallback() noexcept = delete;
  WB_NO_COPY_CTOR_AND_ASSIGNMENT(LookupWithFallback);

  LookupWithFallback(LookupWithFallback&&) noexcept;
  LookupWithFallback& operator=(LookupWithFallback&&) noexcept = delete;

  /**
   * @brief Creates new lookup by locale ids.  If no string found
   * |fallback_string| is used as fallback.
   * @param locale_ids Set of locale ids, order by descending preference.
   * @param fallback_string String to return if requested one not found.
   * @return Lookup.
   */
  [[nodiscard]] static LookupResult<LookupWithFallback> New(
      const std::set<std::string_view>& locale_ids,
      std::string fallback_string = kFallbackString) noexcept;

  /**
   * @brief Gets localized string by message id.  Returns fallback string if one
   * not found.
   * @param message_id Message id.
   * @return Localized string.
   */
  [[nodiscard]] const std::string& String(uint64_t message_id) const noexcept;

  /**
   * @brief Gets localized formatted string by message id.
   * @param message_id Message id.
   * @param format_args Message format args.
   * @return Localized string.
   */
  [[nodiscard]] std::string Format(uint64_t message_id,
                                   fmt::format_args format_args) const noexcept;

  /**
   * @brief Gets string layout.
   * @return StringLayout.
   */
  [[nodiscard]] WB_ATTRIBUTE_CONST StringLayout Layout() const noexcept;

 private:
  Lookup lookup_;
  WB_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()
    // Private member is not accessible to the DLL's client, including inline
    // functions.
    WB_MSVC_DISABLE_WARNING(4251)
    std::string fallback_string_;
  WB_MSVC_END_WARNING_OVERRIDE_SCOPE()

  /**
   * @brief Creates lookup with fallback.
   * @param lookup Lookup.
   * @param fallback_string Fallback string.
   * @return nothing.
   */
  LookupWithFallback(Lookup lookup, std::string fallback_string) noexcept;
};

}  // namespace wb::base::intl

#endif  // !WB_BASE_INTL_LOOKUP_WITH_FALLBACK_H_
