// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// <string> extensions.

#ifndef WB_BASE_STD_EXT_STRING_EXT_H_
#define WB_BASE_STD_EXT_STRING_EXT_H_

#include <string>

#include "base/base_api.h"
#include "build/build_config.h"

namespace wb::base::std_ext {
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
}  // namespace wb::base::std_ext

#endif  // !WB_BASE_STD_EXT_STRING_EXT_H_
