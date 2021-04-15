// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// <string> extensions.

#ifndef WB_BASE_STD2_STRING_EXT_H_
#define WB_BASE_STD2_STRING_EXT_H_

#include <string>

#include "base/config.h"
#include "build/build_config.h"

namespace wb::base::std2 {

#ifdef WB_OS_WIN
/**
 * @brief Wide string to ANSI one.
 * @param in Wide string.
 * @return ANSI string.
 */
[[nodiscard]] WB_BASE_API std::string WideToUTF8(const std::wstring &in);

/**
 * @brief ANSI string to wide one.
 * @param in ANSI string.
 * @return Wide string.
 */
[[nodiscard]] WB_BASE_API std::wstring UTF8ToWide(const std::string &in);
#endif

/**
 * @brief Gets C string or nullptr if empty.
 * @param value String.
 * @return C string or nullptr.
 */
template <typename TElem>
[[nodiscard]] WB_ATTRIBUTE_PURE inline const TElem *zstring_or_null_on_empty(
    const std::basic_string<TElem, std::char_traits<TElem>,
                            std::allocator<TElem>> &value) noexcept {
  return !value.empty() ? value.c_str() : nullptr;
};

}  // namespace wb::base::std2

#endif  // !WB_BASE_STD2_STRING_EXT_H_
