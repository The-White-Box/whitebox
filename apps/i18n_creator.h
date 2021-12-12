// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Creates internationalization lookup.

#ifndef WB_APPS_I18N_CREATOR_H_
#define WB_APPS_I18N_CREATOR_H_

#include "base/intl/lookup_with_fallback.h"
#include "base/intl/scoped_process_locale.h"

namespace wb::apps {

/**
 * @brief Creates internationalization lookup.
 * @param app_name Application name.
 * @param scoped_process_locale Process locale.
 * @return Internationalization lookup.
 */
[[nodiscard]] wb::base::intl::LookupWithFallback CreateIntl(
    std::string_view app_name,
    const wb::base::intl::ScopedProcessLocale& scoped_process_locale) noexcept;

}  // namespace wb::apps

#endif  // WB_APPS_I18N_CREATOR_H_
