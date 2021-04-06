# Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
# Use of this source code is governed by a 3-Clause BSD license that can be
# found in the LICENSE file.
#
# Derived from https://github.com/facebook/folly/blob/master/CMake/FollyConfigChecks.cmake
# which is licensed under the Apache License, Version 2.0 (the "License").  See
# the License for the specific language governing permissions and limitations
# under the License.

string(MAKE_C_IDENTIFIER
  "${CMAKE_BINARY_DIR}/whitebox-config.h.cmake"
  WHITEBOX_CONFIG_HEADER_GUARD_MACRO)
string(TOUPPER
  "${WHITEBOX_CONFIG_HEADER_GUARD_MACRO}"
  WHITEBOX_CONFIG_HEADER_GUARD_MACRO)
set(WHITEBOX_CONFIG_HEADER_GUARD_MACRO "${WHITEBOX_CONFIG_HEADER_GUARD_MACRO}_H_")