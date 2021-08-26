// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Configuration for Windows headers.

#ifndef WB_BASE_WINDOWS_WINDOWS_HEADERS_CONFIG_H_
#define WB_BASE_WINDOWS_WINDOWS_HEADERS_CONFIG_H_

// Prevent tons of unused windows definitions.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// Strict types mode.
#ifndef STRICT
#define STRICT
#endif

// Nobody needs these.
#define NOWINRES
#define NOSERVICE
#define NOMCX
#define NOIME
#ifndef NOMINMAX
#define NOMINMAX
#endif

// Windows 10 features.
#define _WIN32_WINNT 0x0A00

#endif  // !WB_BASE_WINDOWS_WINDOWS_HEADERS_CONFIG_H_
