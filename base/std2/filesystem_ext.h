// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// <filesystem> extensions.

#ifndef WB_BASE_STD2_FILESYSTEM_EXT_H_
#define WB_BASE_STD2_FILESYSTEM_EXT_H_

#include <filesystem>

#include "base/base_api.h"
#include "base/std2/system_error_ext.h"

namespace wb::base::std2::filesystem {

/**
 * Gets path to invoking executable directory.
 * @return Path to executable directory.
 */
[[nodiscard]] WB_BASE_API result<std::filesystem::path>
get_executable_directory() noexcept;

}  // namespace wb::base::std2::filesystem

#endif  // !WB_BASE_STD2_FILESYSTEM_EXT_H_
