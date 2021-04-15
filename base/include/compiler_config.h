// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Compilers configuration and specific things.

#ifndef WHITEBOX_BASE_INCLUDE_COMPILER_CONFIG_H_
#define WHITEBOX_BASE_INCLUDE_COMPILER_CONFIG_H_

#include "build/include/build_config.h"

#ifdef WHITEBOX_COMPILER_MSVC

/*
 * @brief Begins MSVC warning override scope.
 */
#define WHITEBOX_COMPILER_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE() \
  __pragma(warning(push))

/*
 * @brief Disables MSVC warning |warning_level|.
 */
#define WHITEBOX_COMPILER_MSVC_DISABLE_WARNING(warning_level) \
  __pragma(warning(disable : warning_level))

/*
 * @brief Ends MSVC warning override scope.
 */
#define WHITEBOX_COMPILER_MSVC_END_WARNING_OVERRIDE_SCOPE() \
  __pragma(warning(pop))

/*
 * @brief Disable MSVC warning |warning_level| for code |code|.
 */
#define WHITEBOX_COMPILER_MSVC_SCOPED_DISABLE_WARNING(warning_level, code) \
  WHITEBOX_COMPILER_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()                    \
  WHITEBOX_COMPILER_MSVC_DISABLE_WARNING(warning_level)                    \
  code WHITEBOX_COMPILER_MSVC_END_WARNING_OVERRIDE_SCOPE()

#else  // !WHITEBOX_COMPILER_MSVC

#define WHITEBOX_COMPILER_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()
#define WHITEBOX_COMPILER_MSVC_DISABLE_WARNING(warning_level)
#define WHITEBOX_COMPILER_MSVC_END_WARNING_OVERRIDE_SCOPE()
#define WHITEBOX_COMPILER_MSVC_SCOPED_DISABLE_WARNING(warning_level, code) code

#endif  // WHITEBOX_COMPILER_MSVC

#endif  // !WHITEBOX_BASE_INCLUDE_COMPILER_CONFIG_H_
