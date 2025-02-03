// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Enforces a minimum set of policies at build time, ex. the supported compiler
// and library versions.
//
// Derived from abseil base/policy_checks.h.
// See
// https://github.com/abseil/abseil-cpp/blob/master/absl/base/policy_checks.h

#ifndef WB_BASE_POLICY_CHECKS_H_
#define WB_BASE_POLICY_CHECKS_H_

#include <climits>  // CHAR_BIT, INT_MAX
#include <limits>   // std::numeric_limits

// Operating System Checks

#if defined(__CYGWIN__)
#error "Sorry, Cygwin is not supported."
#endif

// Toolchain Checks

// We support MSVC++ Visual Studio 2022 version 17.8.3 and later.
// This minimum will go up.
#if defined(_MSC_FULL_VER) && _MSC_FULL_VER < 193833133 && !defined(__clang__)
#error "This package requires Visual Studio 2022 version 17.8.3 or higher."
#endif

// We support gcc 13.0 and later.
// This minimum will go up.
#if defined(__GNUC__) && !defined(__clang__)
#if __GNUC__ < 13
#error "This package requires gcc 13.0 or higher."
#endif
#endif

// We support clang 15.0 and later.
// This minimum will go up.
#if defined(__clang__)
#if __clang_major__ < 15
#error "This package requires clang 15.0 or higher."
#endif
#endif

// We support Apple Xcode clang 12.0.5 (version 1237602) and later.
// This corresponds to Apple Xcode version 12.5.
// This minimum will go up.
#if defined(__apple_build_version__) && __apple_build_version__ < 1237602
#error "This package requires __apple_build_version__ of 1237602 or higher."
#endif

// C++ Version Check

// Enforce C++20 as the minimum.
#if defined(__cplusplus)
#if __cplusplus < 202002L
#error "C++ versions less than C++20 are not supported."
#endif
#endif

// Standard Library Checks

// POD Types Size Checks

// WhiteBox currently assumes CHAR_BIT == 8.
#if CHAR_BIT != 8
#error "WhiteBox assumes CHAR_BIT == 8."
#endif

// WhiteBox currently assumes that an int is 4 bytes.
#if INT_MAX < 2147483647
#error "WhiteBox assumes that int is at least 4 bytes."
#endif

// WhiteBox currently assumes 4 bytes float.
static_assert(sizeof(float) == 4,  //-V112
              "float should be 4 bytes length. "
              "Please, define 4 bytes float for your platform.");
// WhiteBox expects IEEE 754 float.
static_assert(std::numeric_limits<float>::is_iec559,
              "float should be IEEE 754 / IEC 559");

// WhiteBox currently assumes 8 bytes double.
static_assert(sizeof(double) == 8,
              "double should be 8 bytes length. "
              "Please, define 8 bytes double for your platform.");
// WhiteBox expects IEEE 754 double.
static_assert(std::numeric_limits<double>::is_iec559,
              "double should be IEEE 754 / IEC 559");

#endif  // !WB_BASE_POLICY_CHECKS_H_
