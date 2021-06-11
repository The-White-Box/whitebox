// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Compilers configuration and specific things.

#ifndef WB_BASE_INCLUDE_COMPILER_CONFIG_H_
#define WB_BASE_INCLUDE_COMPILER_CONFIG_H_

#include "build/include/build_config.h"

#ifdef WB_COMPILER_MSVC

/*
 * @brief Begins MSVC warning override scope.
 */
#define WB_COMPILER_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE() \
  __pragma(warning(push))

/*
 * @brief Disables MSVC warning |warning_level|.
 */
#define WB_COMPILER_MSVC_DISABLE_WARNING(warning_level) \
  __pragma(warning(disable : warning_level))

/*
 * @brief Ends MSVC warning override scope.
 */
#define WB_COMPILER_MSVC_END_WARNING_OVERRIDE_SCOPE() \
  __pragma(warning(pop))

/*
 * @brief Disable MSVC warning |warning_level| for code |code|.
 */
#define WB_COMPILER_MSVC_SCOPED_DISABLE_WARNING(warning_level, code) \
  WB_COMPILER_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()                    \
  WB_COMPILER_MSVC_DISABLE_WARNING(warning_level)                    \
  code WB_COMPILER_MSVC_END_WARNING_OVERRIDE_SCOPE()

#else  // !WB_COMPILER_MSVC

#define WB_COMPILER_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()
#define WB_COMPILER_MSVC_DISABLE_WARNING(warning_level)
#define WB_COMPILER_MSVC_END_WARNING_OVERRIDE_SCOPE()
#define WB_COMPILER_MSVC_SCOPED_DISABLE_WARNING(warning_level, code) code

#endif  // WB_COMPILER_MSVC

#endif  // !WB_BASE_INCLUDE_COMPILER_CONFIG_H_
