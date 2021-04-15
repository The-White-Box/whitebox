// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Boot manager API macroses & utilities.

#ifndef WB_BOOT_MANAGER_CONFIG_H_
#define WB_BOOT_MANAGER_CONFIG_H_

#include "build/compiler_config.h"

#if defined(WB_BOOT_MANAGER_DLL) && (WB_BOOT_MANAGER_DLL == 1)
// Export marked symbol from boot manager module.
#define WB_BOOT_MANAGER_API WB_ATTRIBUTE_DLL_EXPORT
#else
// Import marked symbol from boot manager module.
#define WB_BOOT_MANAGER_API WB_ATTRIBUTE_DLL_IMPORT
#endif

#endif  // !WB_BOOT_MANAGER_CONFIG_H_
