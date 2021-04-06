# Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
# Use of this source code is governed by a 3-Clause BSD license that can be
# found in the LICENSE file.
#
# Derived from https://github.com/facebook/folly/blob/master/CMake/FollyFunctions.cmake
# which is licensed under the Apache License, Version 2.0 (the "License").
# See the License for the specific language governing permissions and
# limitations under the License.

option(CLANG_TIDY_DIR_PATH "Define custom clang-tidy directory path." "")
option(GCC_ENABLE_CLANG_TIDY "Determines clang-tidy enabled in GCC builds.  Results in common set of Clang and GCC flags passed to GCC, which can lead to undetected source code warnings." OFF)

# Searches for clang-tidy and applies it for the target.
function(apply_clang_tidy_options_to_target RETURN_VALUE THETARGET)
  unset(${RETURN_VALUE})

  if (NOT CLANG_TIDY_DIR_PATH)
    find_program(CLANG_TIDY
        NAMES "clang-tidy"
        DOC "Path to clang-tidy executable")
  else()
    find_program(CLANG_TIDY
        NAMES "clang-tidy"
        PATHS "${CLANG_TIDY_DIR_PATH}"
        DOC "Path to clang-tidy executable")
  endif()

  if (NOT CLANG_TIDY)
    message(STATUS "clang-tidy not found.")

    set(${RETURN_VALUE} OFF PARENT_SCOPE)
  else()
    message(STATUS "clang-tidy found: ${CLANG_TIDY}, setup one.")

    set(CLANG_TIDY_OPTIONS "${CLANG_TIDY}" "-p=${CMAKE_CURRENT_BINARY_DIR},--header-filter=.,--checks=*,--warnings-as-errors=*")
    set_target_properties(${THETARGET} PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_OPTIONS}" )

    set(${RETURN_VALUE} ON PARENT_SCOPE)
  endif()
endfunction()