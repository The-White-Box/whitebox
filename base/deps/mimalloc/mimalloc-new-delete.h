// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Mimalloc new/delete overrides.

#ifndef WB_BASE_DEPS_MIMALLOC_MIMALLOC_NEW_DELETE_H_
#define WB_BASE_DEPS_MIMALLOC_MIMALLOC_NEW_DELETE_H_

#include "base/deps/mimalloc/mimalloc_config.h"

WB_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()
  WB_MSVC_DISABLE_WARNING(4100)
  WB_MSVC_DISABLE_WARNING(4559)  // 'operator new' : redefinition; the function
                                 // gains __declspec(restrict)
  WB_GCC_BEGIN_WARNING_OVERRIDE_SCOPE()
    WB_GCC_DISABLE_SUGGEST_MALLOC_ATTRIBUTE_WARNING()
#include "deps/mimalloc/include/mimalloc-new-delete.h"
  WB_GCC_END_WARNING_OVERRIDE_SCOPE()
WB_MSVC_END_WARNING_OVERRIDE_SCOPE()

#endif  // !WB_BASE_DEPS_MIMALLOC_MIMALLOC_NEW_DELETE_H_
