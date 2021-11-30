// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// <filesystem> extensions.

#ifndef WB_BASE_STD2_FILESYSTEM_EXT_H_
#define WB_BASE_STD2_FILESYSTEM_EXT_H_

#include <filesystem>
#include <optional>
#include <string_view>

#include "base/config.h"
#include "base/std2/system_error_ext.h"
#include "build/compiler_config.h"

namespace wb::base::std2::filesystem {

/**
 * @brief Gets path to invoking executable directory.
 * @return Path to executable directory.
 */
[[nodiscard]] WB_BASE_API result<std::filesystem::path>
get_executable_directory() noexcept;

#ifdef WB_OS_WIN
/**
 * @brief Extract short exe name from command line.
 * @param command_line Command line.
 * @return Short exe name.
 */
[[nodiscard]] WB_BASE_API std::optional<std::string_view>
get_short_exe_name_from_command_line(std::string_view command_line) noexcept;
#endif

}  // namespace wb::base::std2::filesystem

#endif  // !WB_BASE_STD2_FILESYSTEM_EXT_H_
