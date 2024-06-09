// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// <string_view> extensions.

#ifndef WB_BASE_STD2_STRING_VIEW_EXT_H_
#define WB_BASE_STD2_STRING_VIEW_EXT_H_

#include <cstring>  // std::strlen
#include <string_view>

#include "base/deps/g3log/g3log.h"
#include "build/compiler_config.h"

namespace wb::base::std2 {

/**
 * Is string_view ends with another C string.
 * @param s string_view.
 * @param v C string.
 * @return true if ends, false otherwise.
 */
[[nodiscard]] WB_ATTRIBUTE_PURE inline bool ends_with(
    std::string_view s, const char* WB_MSVC_RESTRICT_VAR v) noexcept {
  G3DCHECK(!!v) << "v is nullptr";
  return v && s.ends_with(v);
}

}  // namespace wb::base::std2

#endif  // !WB_BASE_STD2_STRING_VIEW_EXT_H_
