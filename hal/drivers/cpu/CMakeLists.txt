# Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
# Use of this source code is governed by a 3-Clause BSD license that can be
# found in the LICENSE file.
#
# CPU driver.

cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

set(WB_HAL_CPU_DRIVER_SOURCE_DIR   ${CMAKE_CURRENT_SOURCE_DIR})
set(WB_HAL_CPU_DRIVER_BINARY_DIR   ${CMAKE_CURRENT_BINARY_DIR})
set(WB_HAL_CPU_DRIVER_TARGET_NAME  "whitebox-cpu-driver")

set(WB_HAL_CPU_DRIVER_LINK_OPTIONS "")
set(WB_HAL_CPU_DRIVER_LINK_DEPS
  # Should be first as linker requires it.
  mimalloc
  g3log
  wb::whitebox-base)

if (WB_OS_WIN)
  list(APPEND WB_HAL_CPU_DRIVER_LINK_DEPS mimalloc-redirect)
endif()

wb_cxx_shared_library(
  TARGET        ${WB_HAL_CPU_DRIVER_TARGET_NAME}
  VERSION       ${CMAKE_PROJECT_VERSION}
  DESCRIPTION   "Whitebox HAL CPU Driver"
  SOURCE_DIR    ${WB_HAL_CPU_DRIVER_SOURCE_DIR}
  BINARY_DIR    ${WB_HAL_CPU_DRIVER_BINARY_DIR}
  CXX_DEFS      WB_HAL_CPU_DRIVER_DLL=1
  LINK_OPTS     ${WB_HAL_CPU_DRIVER_LINK_OPTIONS}
  LINK_DEPS     ${WB_HAL_CPU_DRIVER_LINK_DEPS}
)
