// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// G3log configuration.

#ifndef WHITEBOX_BASE_INCLUDE_DEPS_G3LOG_G3LOG_CONFIG_H_
#define WHITEBOX_BASE_INCLUDE_DEPS_G3LOG_G3LOG_CONFIG_H_
#ifdef _WIN32
#pragma once
#endif

#include "base/include/compiler_config.h"
#include "build/include/build_config.h"

#define WHITEBOX_COMPILER_MSVC_BEGIN_GLOG_WARNING_OVERRIDE_SCOPE() \
  WHITEBOX_COMPILER_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()            \
  WHITEBOX_COMPILER_MSVC_DISABLE_WARNING(4820)                     \
  WHITEBOX_COMPILER_MSVC_DISABLE_WARNING(4625)                     \
  WHITEBOX_COMPILER_MSVC_DISABLE_WARNING(4626)                     \
  WHITEBOX_COMPILER_MSVC_DISABLE_WARNING(4355)                     \
  WHITEBOX_COMPILER_MSVC_DISABLE_WARNING(5204)                     \
  WHITEBOX_COMPILER_MSVC_DISABLE_WARNING(5220)                     \
  WHITEBOX_COMPILER_MSVC_DISABLE_WARNING(5026)                     \
  WHITEBOX_COMPILER_MSVC_DISABLE_WARNING(5027)

#define WHITEBOX_COMPILER_MSVC_END_GLOG_WARNING_OVERRIDE_SCOPE() \
  WHITEBOX_COMPILER_MSVC_END_WARNING_OVERRIDE_SCOPE()

#endif  // !WHITEBOX_BASE_INCLUDE_DEPS_G3LOG_G3LOG_CONFIG_H_
