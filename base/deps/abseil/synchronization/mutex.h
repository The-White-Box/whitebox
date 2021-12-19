// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Defines a `Mutex` -- a mutually exclusive lock -- and the most common type of
// synchronization primitive for facilitating locks on shared resources.  A
// mutex is used to prevent multiple threads from accessing and/or writing to a
// shared resource concurrently.

#ifndef WB_BASE_DEPS_ABSEIL_SYNCHRONIZATION_MUTEX_H_
#define WB_BASE_DEPS_ABSEIL_SYNCHRONIZATION_MUTEX_H_

#include "base/deps/abseil/abseil_config.h"

WB_BEGIN_ABSEIL_WARNING_OVERRIDE_SCOPE()
#include "deps/abseil/absl/synchronization/mutex.h"
WB_END_ABSEIL_WARNING_OVERRIDE_SCOPE()

#endif  // !WB_BASE_DEPS_ABSEIL_SYNCHRONIZATION_MUTEX_H_
