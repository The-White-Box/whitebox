// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// CPU Driver API macroses & utilities.

#ifndef WB_HAL_DRIVERS_CPU_CPU_API_H_
#define WB_HAL_DRIVERS_CPU_CPU_API_H_

#include "build/compiler_config.h"

#if defined(WB_HAL_CPU_DRIVER_DLL) && (WB_HAL_CPU_DRIVER_DLL == 1)
// Export marked symbol from HAL CPU driver module.
#define WB_HAL_CPU_DRIVER_API WB_ATTRIBUTE_DLL_EXPORT
#else
// Import marked symbol from HAL CPU driver module.
#define WB_HAL_CPU_DRIVER_API WB_ATTRIBUTE_DLL_IMPORT
#endif

#endif  // !WB_HAL_DRIVERS_CPU_CPU_API_H_
