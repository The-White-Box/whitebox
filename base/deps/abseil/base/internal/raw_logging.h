// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Thread-safe logging routines that do not allocate any memory or
// acquire any locks, and can therefore be used by low-level memory
// allocation, synchronization, and signal-handling code.

#ifndef WB_BASE_DEPS_ABSEIL_BASE_INTERNAL_RAW_LOGGING_H_
#define WB_BASE_DEPS_ABSEIL_BASE_INTERNAL_RAW_LOGGING_H_

#include "base/deps/abseil/abseil_config.h"

WB_BEGIN_ABSEIL_WARNING_OVERRIDE_SCOPE()
#include "deps/abseil/absl/base/internal/raw_logging.h"
WB_END_ABSEIL_WARNING_OVERRIDE_SCOPE()

#endif  // !WB_BASE_DEPS_ABSEIL_BASE_INTERNAL_RAW_LOGGING_H_