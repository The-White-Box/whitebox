// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Provides base module API macroses.

#ifndef WB_BASE_BASE_API_H_
#define WB_BASE_BASE_API_H_

#include "build/compiler_config.h"

#if defined(WB_BASE_DLL) && (WB_BASE_DLL == 1)
// Export marked symbol from base module.
#define WB_BASE_API WB_ATTRIBUTE_DLL_EXPORT
#else
// Import marked symbol from base module.
#define WB_BASE_API WB_ATTRIBUTE_DLL_IMPORT
#endif

#endif  // !WB_BASE_BASE_API_H_
