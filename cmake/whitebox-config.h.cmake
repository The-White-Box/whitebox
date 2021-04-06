// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.

#ifndef @WHITEBOX_CONFIG_HEADER_GUARD_MACRO@
#define @WHITEBOX_CONFIG_HEADER_GUARD_MACRO@

#cmakedefine WHITEBOX_HAVE_PTHREAD 1

namespace whitebox {
namespace project {
constexpr char kName[]{"@PROJECT_NAME@"};

namespace version {
constexpr char kFull[]{"@PROJECT_VERSION@"};
constexpr char kMajor[]{"@PROJECT_VERSION_MAJOR@"};
constexpr char kMinor[]{"@PROJECT_VERSION_MINOR@"};
constexpr char kPatch[]{"@PROJECT_VERSION_PATCH@"};
}  // namespace version
}  // namespace project
}  // namespace whitebox

#endif  // !@WHITEBOX_CONFIG_HEADER_GUARD_MACRO@