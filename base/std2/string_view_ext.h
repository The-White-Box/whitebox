// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// <string_view> extensions.

#ifndef WB_BASE_STD2_STRING_VIEW_EXT_H_
#define WB_BASE_STD2_STRING_VIEW_EXT_H_

#include <string_view>

#include "base/config.h"
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

/**
 * @brief Trim spaces around data.
 * @param in Input to trim spaces around.
 * @param out Output to write |in| with trimmed spaces to.
 * @param out_size Output buffer size.
 * @return true if smth trimmed, false otherwise.
 */
WB_BASE_API bool TrimSpaces(std::string_view in, char* out,
                            std::size_t out_size) noexcept;

/**
 * @brief Trim spaces around data.
 * @param in Input to trim spaces around.
 * @param out Output to write |in| with trimmed spaces to.
 * @return true if smth trimmed, false otherwise.
 */
template <size_t size>
inline bool TrimSpaces(std::string_view in, char (&out)[size]) noexcept {
  return TrimSpaces(in, out, sizeof(out));
}

}  // namespace wb::base::std2

#endif  // !WB_BASE_STD2_STRING_VIEW_EXT_H_
