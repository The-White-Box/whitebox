// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Provides an API for looking up localized message strings.
//
// Usage example:
//
// const std::string &hello_message{intl::l18n(intl, "Hello, international
// world!")};
//
// or
//
// const std::string greetings{intl::l18n_fmt(intl, "Hi, {0}!", user_name)};
//
// Based on
// https://cs.opensource.google/fuchsia/fuchsia/+/main:src/lib/intl/lookup/cpp/lookup.h

#include "base/intl/lookup_with_fallback.h"

#include "base/deps/g3log/g3log.h"

namespace wb::base::intl {

LookupWithFallback::LookupWithFallback(LookupWithFallback&& l) noexcept
    : lookup_{std::move(l.lookup_)},
      fallback_string_{std::move(l.fallback_string_)} {}

[[nodiscard]] LookupResult<LookupWithFallback> LookupWithFallback::New(
    const std::set<std::string_view>& locale_ids,
    std::string fallback_string) noexcept {
  auto lookup_result = Lookup::New(locale_ids);
  if (auto* lookup = std::get_if<Lookup>(&lookup_result)) WB_ATTRIBUTE_LIKELY {
      return LookupResult<LookupWithFallback>{
          LookupWithFallback(std::move(*lookup), std::move(fallback_string))};
    }

  const auto* status = std::get_if<Status>(&lookup_result);
  G3DCHECK(!!status);
  return LookupResult<LookupWithFallback>{*status};
}

[[nodiscard]] const std::string& LookupWithFallback::String(
    uint64_t message_id) const noexcept {
  auto result = lookup_.String(message_id);
  if (const auto* string = std::get_if<Lookup::Ref<const std::string>>(&result))
    WB_ATTRIBUTE_LIKELY { return *string; }

  G3LOG(WARNING) << "Missed localization string for " << message_id
                 << " message id.";
  return fallback_string_;
}

[[nodiscard]] std::string LookupWithFallback::Format(
    uint64_t message_id, fmt::format_args format_args) const noexcept {
  auto result = lookup_.Format(message_id, format_args);
  if (const auto* string = std::get_if<std::string>(&result))
    WB_ATTRIBUTE_LIKELY { return fmt::vformat(*string, format_args); }

  G3LOG(WARNING) << "Missed localization string for " << message_id
                 << " message id.";
  return fallback_string_;
}

[[nodiscard]] WB_ATTRIBUTE_CONST StringLayout
LookupWithFallback::Layout() const noexcept {
  return lookup_.Layout();
}

LookupWithFallback::LookupWithFallback(Lookup lookup,
                                       std::string fallback_string) noexcept
    : lookup_{std::move(lookup)},
      fallback_string_{std::move(fallback_string)} {}

}  // namespace wb::base::intl
