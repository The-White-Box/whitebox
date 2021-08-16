// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Human Interface Device Driver API macroses & utilities.

#ifndef WB_HAL_DRIVERS_HID_HID_API_H_
#define WB_HAL_DRIVERS_HID_HID_API_H_

#include "build/compiler_config.h"

#if defined(WB_HAL_HID_DRIVER_DLL) && (WB_HAL_HID_DRIVER_DLL == 1)
// Export marked symbol from HAL HID driver module.
#define WB_HAL_HID_DRIVER_API WB_ATTRIBUTE_DLL_EXPORT
#else
// Import marked symbol from HAL HID driver module.
#define WB_HAL_HID_DRIVER_API WB_ATTRIBUTE_DLL_IMPORT
#endif

#endif  // !WB_HAL_DRIVERS_HID_HID_API_H_
