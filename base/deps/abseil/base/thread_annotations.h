// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Macro definitions for thread safety annotations that allow developers to
// document the locking policies of multi-threaded code.  The annotations can
// also help program analysis tools to identify potential thread safety issues.

#ifndef WB_BASE_DEPS_ABSEIL_BASE_THREAD_ANNOTATIONS_H_
#define WB_BASE_DEPS_ABSEIL_BASE_THREAD_ANNOTATIONS_H_

#include "base/deps/abseil/abseil_config.h"

WB_BEGIN_ABSEIL_WARNING_OVERRIDE_SCOPE()
#include "deps/abseil/absl/base/thread_annotations.h"
WB_END_ABSEIL_WARNING_OVERRIDE_SCOPE()

#endif  // !WB_BASE_DEPS_ABSEIL_BASE_THREAD_ANNOTATIONS_H_