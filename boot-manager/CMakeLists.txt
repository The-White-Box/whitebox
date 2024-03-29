# Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
# Use of this source code is governed by a 3-Clause BSD license that can be
# found in the LICENSE file.
#
# Boot manager library project definition.

cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

set(WB_BOOTMGR_SOURCE_DIR   ${CMAKE_CURRENT_SOURCE_DIR})
set(WB_BOOTMGR_BINARY_DIR   ${CMAKE_CURRENT_BINARY_DIR})
set(WB_BOOTMGR_TARGET_NAME  "whitebox-boot-manager")

set(WB_BOOTMGR_LINK_OPTIONS "")

set(WB_BOOTMGR_LINK_DEPS
  # Should be first as linker requires it.
  mimalloc
  marl
  absl::strings
  g3log
  wb::whitebox-base
  wb::whitebox-ui)
if (WB_OS_WIN)
  list(APPEND WB_BOOTMGR_LINK_DEPS "Winmm")
elseif (WB_OS_LINUX)
  # dlopen, shm_open, filesystem.
  list(APPEND WB_BOOTMGR_LINK_DEPS "dl;rt;stdc++fs")
elseif (WB_OS_MACOS)
  # dlopen, shm_open, filesystem.
  list(APPEND WB_BOOTMGR_LINK_DEPS "dl;rt;c++fs")
endif()

set(WB_BOOTMGR_RUNTIME_DEPS
  # Should be first as linker requires it.
  mimalloc
  marl
  g3log
  wb::whitebox-base
  wb::whitebox-kernel
  wb::whitebox-ui)

wb_cxx_shared_library(
  TARGET        ${WB_BOOTMGR_TARGET_NAME}
  VERSION       ${CMAKE_PROJECT_VERSION}
  DESCRIPTION   "Whitebox Boot Manager"
  SOURCE_DIR    ${WB_BOOTMGR_SOURCE_DIR}
  BINARY_DIR    ${WB_BOOTMGR_BINARY_DIR}
  CXX_DEFS      WB_BOOT_MANAGER_DLL=1
  LINK_OPTS     ${WB_BOOTMGR_LINK_OPTIONS}
  LINK_DEPS     ${WB_BOOTMGR_LINK_DEPS}
  RUNTIME_DEPS  ${WB_BOOTMGR_RUNTIME_DEPS}
)
