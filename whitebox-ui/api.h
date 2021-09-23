// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Whitebox UI API macroses & utilities.

#ifndef WB_WHITEBOX_UI_API_H_
#define WB_WHITEBOX_UI_API_H_

#include "build/compiler_config.h"

#if defined(WB_WHITEBOX_UI_DLL) && (WB_WHITEBOX_UI_DLL == 1)
#define WB_WHITEBOX_UI_API WB_ATTRIBUTE_DLL_EXPORT
#else
#define WB_WHITEBOX_UI_API WB_ATTRIBUTE_DLL_IMPORT
#endif

#endif  // !WB_WHITEBOX_UI_API_H_
