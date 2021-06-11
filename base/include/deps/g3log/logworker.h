// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// G3log log worker.

#ifndef WB_BASE_INCLUDE_DEPS_G3LOG_LOGWORKER_H_
#define WB_BASE_INCLUDE_DEPS_G3LOG_LOGWORKER_H_
#ifdef _WIN32
#pragma once
#endif

#include "g3log_config.h"

WB_COMPILER_MSVC_BEGIN_GLOG_WARNING_OVERRIDE_SCOPE()
#include "deps/g3log/src/g3log/logworker.hpp"
WB_COMPILER_MSVC_END_GLOG_WARNING_OVERRIDE_SCOPE()

#endif  // !WB_BASE_INCLUDE_DEPS_G3LOG_LOGWORKER_H_
