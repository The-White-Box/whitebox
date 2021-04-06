# Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
# Use of this source code is governed by a 3-Clause BSD license that can be
# found in the LICENSE file.

cmake_minimum_required (VERSION 3.8 FATAL_ERROR)

set(TARGET_NAME "half-life-2")

project(${TARGET_NAME}
  VERSION ${CMAKE_PROJECT_VERSION}
  DESCRIPTION "Half-Life 2"
  LANGUAGES CXX)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# We currently build app by finding all sources and header files.
# We then exclude specific files below.
auto_sources(files "*.cc" "RECURSE" "${SOURCE_DIR}")
auto_sources(hfiles "*.h" "RECURSE" "${SOURCE_DIR}")

# Add sources to this project's executable.
add_executable(${TARGET_NAME} WIN32 ${files} ${hfiles})

# Specify project compile / link options.
apply_compile_options_to_target(${TARGET_NAME})

# Include the directory with the new files.
target_include_directories(${TARGET_NAME}
  PUBLIC
    ${CMAKE_BINARY_DIR}
    ${WHITEBOX_ROOT_PATH}
)
