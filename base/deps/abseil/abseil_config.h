// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Abseil configuration.

#ifndef WB_BASE_DEPS_ABSEIL_ABSEIL_CONFIG_H_
#define WB_BASE_DEPS_ABSEIL_ABSEIL_CONFIG_H_

#include "build/compiler_config.h"

/**
 * @brief Begin Abseil warning override scope.
 */
#define WB_BEGIN_ABSEIL_WARNING_OVERRIDE_SCOPE() \
  WB_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()         \
    WB_MSVC_DISABLE_WARNING(4324)                \
    WB_MSVC_DISABLE_WARNING(4365)                \
    WB_MSVC_DISABLE_WARNING(4623)                \
    WB_MSVC_DISABLE_WARNING(4625)                \
    WB_MSVC_DISABLE_WARNING(4626)                \
    WB_MSVC_DISABLE_WARNING(4820)                \
    WB_MSVC_DISABLE_WARNING(5026)                \
    WB_MSVC_DISABLE_WARNING(5027)                \
    WB_GCC_BEGIN_WARNING_OVERRIDE_SCOPE()        \
      WB_GCC_DISABLE_DOUBLE_PROMOTION_WARNING()  \
      WB_GCC_DISABLE_PADDED_WARNING()            \
      WB_GCC_DISABLE_SIGN_CONVERSION_WARNING()

/**
 * @brief End Abseil warning override scope.
 */
#define WB_END_ABSEIL_WARNING_OVERRIDE_SCOPE() \
  WB_GCC_END_WARNING_OVERRIDE_SCOPE            \
  ()                                           \
  WB_MSVC_END_WARNING_OVERRIDE_SCOPE           \
  ()

#endif  // !WB_BASE_DEPS_ABSEIL_ABSEIL_CONFIG_H_
