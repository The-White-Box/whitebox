// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Bootmgr API macroses & utilities.

#ifndef WB_BOOTMGR_API_H_
#define WB_BOOTMGR_API_H_

#include "build/compiler_config.h"

#if defined(WB_BOOTMGR_DLL) && (WB_BOOTMGR_DLL == 1)
// Export marked symbol from bootmgr module.
#define WB_BOOTMGR_API WB_ATTRIBUTE_DLL_EXPORT
#else
// Import marked symbol from bootmgr module.
#define WB_BOOTMGR_API WB_ATTRIBUTE_DLL_IMPORT
#endif

#endif  // !WB_BOOTMGR_API_H_
