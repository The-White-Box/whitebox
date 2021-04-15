// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// G3log configuration.

#ifndef WB_BASE_DEPS_G3LOG_G3LOG_CONFIG_H_
#define WB_BASE_DEPS_G3LOG_G3LOG_CONFIG_H_

#include "build/build_config.h"
#include "build/compiler_config.h"

#define WB_BEGIN_G3LOG_WARNING_OVERRIDE_SCOPE()    \
  WB_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()           \
    WB_MSVC_DISABLE_WARNING(4100)                  \
    WB_MSVC_DISABLE_WARNING(4355)                  \
    WB_MSVC_DISABLE_WARNING(4625)                  \
    WB_MSVC_DISABLE_WARNING(4626)                  \
    WB_MSVC_DISABLE_WARNING(4820)                  \
    WB_MSVC_DISABLE_WARNING(5204)                  \
    WB_MSVC_DISABLE_WARNING(5220)                  \
    WB_MSVC_DISABLE_WARNING(5263)                  \
    WB_MSVC_DISABLE_WARNING(5026)                  \
    WB_MSVC_DISABLE_WARNING(5027)                  \
    WB_GCC_BEGIN_WARNING_OVERRIDE_SCOPE() \
      WB_GCC_DISABLE_PADDED_WARNING()

#define WB_END_G3LOG_WARNING_OVERRIDE_SCOPE() \
  WB_GCC_END_WARNING_OVERRIDE_SCOPE  \
  ()                                          \
  WB_MSVC_END_WARNING_OVERRIDE_SCOPE          \
  ()

#endif  // !WB_BASE_DEPS_G3LOG_G3LOG_CONFIG_H_
