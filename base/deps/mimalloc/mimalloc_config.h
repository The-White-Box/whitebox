// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Mimalloc configuration.

#ifndef WB_BASE_DEPS_MIMALLOC_MIMALLOC_CONFIG_H_
#define WB_BASE_DEPS_MIMALLOC_MIMALLOC_CONFIG_H_

#include "build/compiler_config.h"

#define WB_BEGIN_MIMALLOC_WARNING_OVERRIDE_SCOPE() \
  WB_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()           \
    WB_MSVC_DISABLE_WARNING(4820)                  \
    WB_GCC_BEGIN_WARNING_OVERRIDE_SCOPE()

#define WB_END_MIMALLOC_WARNING_OVERRIDE_SCOPE() \
  WB_GCC_END_WARNING_OVERRIDE_SCOPE              \
  ()                                             \
  WB_MSVC_END_WARNING_OVERRIDE_SCOPE             \
  ()

#endif  // !WB_BASE_DEPS_MIMALLOC_MIMALLOC_CONFIG_H_
