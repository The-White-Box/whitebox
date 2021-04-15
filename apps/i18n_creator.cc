// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Creates internationalization lookup.

#include "i18n_creator.h"

#include <string>
#include <utility>  // std::move

#include "base/deps/g3log/g3log.h"

namespace wb::apps {

/**
 * @brief Creates internationalization lookup.
 * @param app_name Application name.
 * @param scoped_process_locale Process locale.
 * @return Internationalization lookup.
 */
[[nodiscard]] wb::base::intl::LookupWithFallback CreateIntl(
    std::string_view app_name,
    const wb::base::intl::ScopedProcessLocale& scoped_process_locale) noexcept {
  using namespace wb::base::intl;

  const std::optional<std::string> maybe_user_locale{
      scoped_process_locale.GetCurrentLocale()};
  G3LOG_IF(WARNING, !maybe_user_locale.has_value())
      << app_name << " unable to use UTF-8 locale '" << locales::kUtf8Locale
      << "' for UI, fallback to '" << locales::kFallbackLocale << "'.";

  const std::string user_locale{
      maybe_user_locale.value_or(locales::kFallbackLocale)};
  G3LOG(INFO) << app_name << " using " << user_locale << " locale for UI.";

  auto intl_lookup_result{LookupWithFallback::New({user_locale})};
  auto intl_lookup = std::get_if<LookupWithFallback>(&intl_lookup_result);

  G3LOG_IF(FATAL, !intl_lookup)
      << "Unable to create localization strings lookup for locale "
      << user_locale << ".";
  return std::move(*intl_lookup);
}

}  // namespace wb::apps
