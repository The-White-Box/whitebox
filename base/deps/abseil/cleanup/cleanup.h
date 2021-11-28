// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// `absl::Cleanup` implements the scope guard idiom, invoking the contained
// callback's `operator()() &&` on scope exit.

#ifndef WB_BASE_DEPS_ABSEIL_CLEANUP_CLEANUP_H_
#define WB_BASE_DEPS_ABSEIL_CLEANUP_CLEANUP_H_

#include "base/deps/abseil/abseil_config.h"

WB_BEGIN_ABSEIL_WARNING_OVERRIDE_SCOPE()
#include "deps/abseil/absl/cleanup/cleanup.h"
WB_END_ABSEIL_WARNING_OVERRIDE_SCOPE()

#endif  // !WB_BASE_DEPS_ABSEIL_CLEANUP_CLEANUP_H_
