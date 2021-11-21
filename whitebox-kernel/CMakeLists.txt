# Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
# Use of this source code is governed by a 3-Clause BSD license that can be
# found in the LICENSE file.
#
# WhiteBox kernel library project definition.

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

set(WB_WHITEBOX_KERNEL_SOURCE_DIR   ${CMAKE_CURRENT_SOURCE_DIR})
set(WB_WHITEBOX_KERNEL_BINARY_DIR   ${CMAKE_CURRENT_BINARY_DIR})
set(WB_WHITEBOX_KERNEL_TARGET_NAME  "whitebox-kernel")

set(WB_WHITEBOX_KERNEL_LINK_OPTIONS "")
set(WB_WHITEBOX_KERNEL_LINK_DEPS
  absl::strings
  fmt
  g3log
  mimalloc
  wb::whitebox-base
  wb::whitebox-hid-driver
  wb::whitebox-ui)
if (WB_OS_WIN)
  list(APPEND WB_WHITEBOX_KERNEL_LINK_DEPS Dwmapi)
else ()
  list(APPEND WB_WHITEBOX_KERNEL_LINK_DEPS "SDL2::SDL2" "SDL2::image")
endif ()

set(WB_WHITEBOX_KERNEL_RUNTIME_DEPS
  g3log
  mimalloc
  wb::whitebox-base
  wb::whitebox-hid-driver
  wb::whitebox-ui)
if (NOT WB_OS_WIN)
  list(APPEND WB_WHITEBOX_KERNEL_RUNTIME_DEPS "SDL2::SDL2" "SDL2::image")
endif ()

wb_cxx_shared_library(
  TARGET        ${WB_WHITEBOX_KERNEL_TARGET_NAME}
  VERSION       ${CMAKE_PROJECT_VERSION}
  DESCRIPTION   "Whitebox Lightweight Kernel"
  SOURCE_DIR    ${WB_WHITEBOX_KERNEL_SOURCE_DIR}
  BINARY_DIR    ${WB_WHITEBOX_KERNEL_BINARY_DIR}
  CXX_DEFS      WB_WHITEBOX_KERNEL_DLL=1
  LINK_DEPS     ${WB_WHITEBOX_KERNEL_LINK_DEPS}
  RUNTIME_DEPS  ${WB_WHITEBOX_KERNEL_RUNTIME_DEPS}
)
