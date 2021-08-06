// Copyright (c) 2021 The win32docs Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.

#ifndef WB_BASE_STD_EXT_STRING_VIEW_EXT_H_
#define WB_BASE_STD_EXT_STRING_VIEW_EXT_H_

#include <cstring>
#include <string_view>

#include "base/deps/g3log/g3log.h"
#include "build/compiler_config.h"

namespace wb::base::std_ext {
/**
 * Is string_view starts with character.
 * @param s string_view.
 * @param c Character.
 * @return true if starts, false otherwise.
 */
[[nodiscard]] WB_ATTRIBUTE_GCC_PURE bool starts_with(std::string_view s,
                                                     char c) noexcept {
  return !s.empty() && s[0] == c;
}

/**
 * Is string_view ends with character.
 * @param s string_view.
 * @param c Character.
 * @return true if ends, false otherwise.
 */
[[nodiscard]] WB_ATTRIBUTE_GCC_PURE bool ends_with(std::string_view s,
                                                   char c) noexcept {
  return !s.empty() && s[s.size() - 1] == c;
}

/**
 * Is string_view ends with another C string.
 * @param s string_view.
 * @param v C string.
 * @return true if ends, false otherwise.
 */
[[nodiscard]] WB_ATTRIBUTE_GCC_PURE bool ends_with(std::string_view s,
                                                   const char *v) noexcept {
  G3DCHECK(!!v);
  const auto idx = s.find_last_of(v);
  return idx == s.size() - std::strlen(v);
}
}  // namespace wb::base::std_ext

#endif  // !WB_BASE_STD_EXT_STRING_VIEW_EXT_H_
