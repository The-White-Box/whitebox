// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Command line switches.

#ifndef WB_BASE_BASE_SWITCHES_H_
#define WB_BASE_BASE_SWITCHES_H_

#include "base/deps/abseil/flags/declare.h"

// Insecure.  Allow to load NOT SIGNED module targets.  There is no guarantee
// unsigned module doing nothing harmful.  Use at your own risk, ex. for
// debugging or mods.
ABSL_DECLARE_FLAG(bool, insecure_allow_unsigned_module_target);

#endif  // !WB_BASE_BASE_SWITCHES_H_
