// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Light version of Windows header.

#ifndef WB_BASE_INCLUDE_WINDOWS_WINDOWS_LIGHT_H_
#define WB_BASE_INCLUDE_WINDOWS_WINDOWS_LIGHT_H_
#ifdef _WIN32
#pragma once
#endif

#include "base/include/compiler_config.h"
#include "windows_headers_config.h"  // Presetup Windows.h and friends.
//
#include <Windows.h>
// Os version helpers.
#include <Versionhelpers.h>
// Fixup clashing windows macroses.
#include "windows_macroses_fixup.h"

#endif  // !WB_BASE_INCLUDE_WINDOWS_WINDOWS_LIGHT_H_
