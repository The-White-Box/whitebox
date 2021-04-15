// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Configuration for Windows headers.

#ifndef WHITEBOX_BASE_INCLUDE_WINDOWS_WINDOWS_HEADERS_CONFIG_H_
#define WHITEBOX_BASE_INCLUDE_WINDOWS_WINDOWS_HEADERS_CONFIG_H_
#ifdef _WIN32
#pragma once
#endif

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
#define NOMINMAX

// Windows 10 features.
#define _WIN32_WINNT 0x0A00

#endif  // !WHITEBOX_BASE_INCLUDE_WINDOWS_WINDOWS_HEADERS_CONFIG_H_
