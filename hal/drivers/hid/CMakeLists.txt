# Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
# Use of this source code is governed by a 3-Clause BSD license that can be
# found in the LICENSE file.
#
# Human Interface Device driver.

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

set(WB_HAL_HID_DRIVER_SOURCE_DIR   ${CMAKE_CURRENT_SOURCE_DIR})
set(WB_HAL_HID_DRIVER_BINARY_DIR   ${CMAKE_CURRENT_BINARY_DIR})
set(WB_HAL_HID_DRIVER_TARGET_NAME  "whitebox-hid-driver")

set(WB_HAL_HID_DRIVER_LINK_OPTIONS "")
set(WB_HAL_HID_DRIVER_LINK_DEPS g3log mimalloc wb::whitebox-base)
set(WB_HAL_HID_DRIVER_RUNTIME_DEPS g3log mimalloc wb::whitebox-base)

wb_cxx_shared_library(
  TARGET        ${WB_HAL_HID_DRIVER_TARGET_NAME}
  VERSION       ${CMAKE_PROJECT_VERSION}
  DESCRIPTION   "Whitebox HAL Human Interface Device Driver"
  SOURCE_DIR    ${WB_HAL_HID_DRIVER_SOURCE_DIR}
  BINARY_DIR    ${WB_HAL_HID_DRIVER_BINARY_DIR}
  CXX_DEFS      WB_HAL_HID_DRIVER_DLL=1
  LINK_OPTS     ${WB_HAL_HID_DRIVER_LINK_OPTIONS}
  LINK_DEPS     ${WB_HAL_HID_DRIVER_LINK_DEPS}
  RUNTIME_DEPS  ${WB_HAL_HID_DRIVER_RUNTIME_DEPS}
)
