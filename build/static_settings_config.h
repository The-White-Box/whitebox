// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Static settings configuration.

#ifndef WB_BUILD_STATIC_SETTINGS_CONFIG_H_
#define WB_BUILD_STATIC_SETTINGS_CONFIG_H_

#include "build/build_config.h"

namespace wb::build::settings {

#if defined(WB_OS_WIN)
/**
 * @brief Path to main log file.
 */
constexpr char kPathToMainLogFile[]{""};
#elif defined(WB_OS_POSIX)
/**
 * @brief Path to main log file.
 */
constexpr char kPathToMainLogFile[]{"/tmp/"};
#else
#error "Please define path to main log file for your platform."
#endif

namespace ui::window::dimensions {
// https://www.microsoft.com/en-us/windows/windows-10-specifications
// On Windows 10 minimum display dimensions are 800x600, so do not use more than.
constexpr int kMinWidth{640};
constexpr int kMinHeight{480};
}  // namespace ui

}  // namespace wb::build::settings

#endif  // !WB_BUILD_STATIC_SETTINGS_CONFIG_H_
