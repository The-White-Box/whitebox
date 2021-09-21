// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// fmt configuration.

#ifndef WB_BASE_DEPS_FMT_CONFIG_H_
#define WB_BASE_DEPS_FMT_CONFIG_H_

#define WB_BEGIN_FMT_WARNING_OVERRIDE_SCOPE() \
  WB_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()      \
    WB_MSVC_DISABLE_WARNING(4582)             \
    WB_MSVC_DISABLE_WARNING(4625)             \
    WB_MSVC_DISABLE_WARNING(4626)             \
    WB_MSVC_DISABLE_WARNING(4820)             \
    WB_MSVC_DISABLE_WARNING(4355)             \
    WB_MSVC_DISABLE_WARNING(4927)             \
    WB_MSVC_DISABLE_WARNING(5027)             \
    WB_GCC_BEGIN_WARNING_OVERRIDE_SCOPE()     \
      WB_GCC_DISABLE_PADDED_WARNING()         \
      WB_GCC_DISABLE_MISSING_NORETURN_WARNING()

#define WB_END_FMT_WARNING_OVERRIDE_SCOPE() \
  WB_GCC_END_WARNING_OVERRIDE_SCOPE         \
  ()                                        \
  WB_MSVC_END_WARNING_OVERRIDE_SCOPE        \
  ()

#endif  // !WB_BASE_DEPS_FMT_CONFIG_H_
