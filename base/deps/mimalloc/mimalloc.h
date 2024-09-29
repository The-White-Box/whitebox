// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Mimalloc entry point.

#ifndef WB_BASE_DEPS_MIMALLOC_MIMALLOC_H_
#define WB_BASE_DEPS_MIMALLOC_MIMALLOC_H_

#include "base/deps/mimalloc/mimalloc_config.h"

WB_BEGIN_MIMALLOC_WARNING_OVERRIDE_SCOPE()
#include "deps/mimalloc/include/mimalloc.h"
WB_END_MIMALLOC_WARNING_OVERRIDE_SCOPE()

#endif  // !WB_BASE_DEPS_MIMALLOC_MIMALLOC_H_
