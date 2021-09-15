// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// <string_view> extensions.

#ifndef WB_BASE_STD_EXT_STRING_VIEW_EXT_H_
#define WB_BASE_STD_EXT_STRING_VIEW_EXT_H_

#include <cstring>  // std::strlen
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
[[nodiscard]] WB_ATTRIBUTE_GCC_PURE inline bool starts_with(std::string_view s,
                                                            char c) noexcept {
  return !s.empty() && s[0] == c;
}

/**
 * Is string_view ends with character.
 * @param s string_view.
 * @param c Character.
 * @return true if ends, false otherwise.
 */
[[nodiscard]] WB_ATTRIBUTE_GCC_PURE inline bool ends_with(std::string_view s,
                                                          char c) noexcept {
  return !s.empty() && s[s.size() - 1] == c;
}

/**
 * Is string_view ends with another C string.
 * @param s string_view.
 * @param v C string.
 * @return true if ends, false otherwise.
 */
[[nodiscard]] WB_ATTRIBUTE_GCC_PURE inline bool ends_with(
    std::string_view s, const char* WB_COMPILER_MSVC_RESTRICT_VAR v) noexcept {
  G3DCHECK(!!v) << "v is nullptr";
#if WB_COMPILER_HAS_CXX20
  return v && s.ends_with(v);
#else
  if (!v) [[unlikely]] {
    return false;
  }
  const auto idx = s.rfind(v);
  return idx != std::string_view::npos && idx == (s.size() - std::strlen(v));
#endif
}
}  // namespace wb::base::std_ext

#endif  // !WB_BASE_STD_EXT_STRING_VIEW_EXT_H_
