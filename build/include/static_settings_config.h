// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Product version info configuration.

#ifndef BUILD_INCLUDE_STATIC_SETTINGS_CONFIG_H_
#define BUILD_INCLUDE_STATIC_SETTINGS_CONFIG_H_
#ifdef _WIN32
#pragma once
#endif

#include "build_config.h"

namespace whitebox::build::settings {
#if defined(WHITEBOX_OS_WIN)
/**
 * @brief Path to main log file.
 */
constexpr char kPathToMainLogFile[]{"./logs/"};
#elif defined(WHITEBOX_OS_POSIX)
/**
 * @brief Path to main log file.
 */
constexpr char kPathToMainLogFile[]{"/tmp/"};
#else
#error Please define path to main log file for your platform in build/include/static_settings_config.h.
#endif

/**
 * @brief Minimum timers resolution.  Good enough, but not too power hungry.
 */
constexpr unsigned kMinimumTimerResolutionMs{8U};
}  // namespace whitebox::build::settings

#endif  // !BUILD_INCLUDE_STATIC_SETTINGS_CONFIG_H_
