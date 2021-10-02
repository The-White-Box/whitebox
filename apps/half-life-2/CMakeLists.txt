# Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
# Use of this source code is governed by a 3-Clause BSD license that can be
# found in the LICENSE file.
#
# Half-Life 2 launcher project definition.

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

set(WB_HALF_LIFE2_SOURCE_DIR   ${CMAKE_CURRENT_SOURCE_DIR})
set(WB_HALF_LIFE2_BINARY_DIR   ${CMAKE_CURRENT_BINARY_DIR})
set(WB_HALF_LIFE2_TARGET_NAME  "half-life-2")

set(WB_HALF_LIFE2_LINK_OPTIONS "")

set(WB_HALF_LIFE2_LINK_DEPS fmt g3log mimalloc
  wb::whitebox-base wb::whitebox-ui)
if (WB_OS_WIN)
  # _com_ptr_t support.
  list(APPEND WB_HALF_LIFE2_LINK_DEPS "comsuppw")
elseif (WB_OS_LINUX)
  # dlopen, filesystem.
  list(APPEND WB_HALF_LIFE2_LINK_DEPS "dl;stdc++fs")
elseif (WB_OS_MACOS)
  # dlopen, filesystem.
  list(APPEND WB_HALF_LIFE2_LINK_DEPS "dl;c++fs")
endif()

set(WB_HALF_LIFE2_RUNTIME_DEPS
    g3log mimalloc
    wb::whitebox-boot-manager
    wb::whitebox-base
    wb::whitebox-hid-driver
    wb::whitebox-kernel
    wb::whitebox-ui)
if (NOT WB_OS_WIN)
  list(APPEND WB_HALF_LIFE2_LINK_DEPS wb::whitebox-base SDL2::SDL2)
  list(APPEND WB_HALF_LIFE2_RUNTIME_DEPS
    "png" "wb::whitebox-base" "SDL2::SDL2" "SDL2::image")
endif()

wb_cxx_executable(
  PROJECT_NAME  "Half-Life 2" 
  TARGET        ${WB_HALF_LIFE2_TARGET_NAME}
  VERSION       ${CMAKE_PROJECT_VERSION}
  DESCRIPTION   "Half-Life 2"
  SOURCE_DIR    ${WB_HALF_LIFE2_SOURCE_DIR}
  BINARY_DIR    ${WB_HALF_LIFE2_BINARY_DIR}
  LINK_OPTS     ${WB_HALF_LIFE2_LINK_OPTIONS}
  LINK_DEPS     ${WB_HALF_LIFE2_LINK_DEPS}
  RUNTIME_DEPS  ${WB_HALF_LIFE2_RUNTIME_DEPS}
)

if (NOT WB_OS_WIN)
  wb_copy_target_asset_to_target_bin_dir(${WB_HALF_LIFE2_TARGET_NAME}
    "images/Half-Life 2 icon.png")
endif()
