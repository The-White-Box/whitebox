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
  list(APPEND WB_BOOTMGR_LINK_DEPS mimalloc-redirect Winmm)
elseif (WB_OS_LINUX)
  # dlopen, shm_open, filesystem.
  list(APPEND WB_BOOTMGR_LINK_DEPS "dl;stdc++fs")
elseif (WB_OS_MACOS)
  # dlopen, filesystem.
  list(APPEND WB_BOOTMGR_LINK_DEPS "dl")
endif()

if (WB_OS_LINUX OR WB_OS_MACOS)
  # Not needed on modern systems, but still.
  find_library(WB_LIB_RT rt)
  find_library(WB_LIB_CPP_FS c++fs)

  if (WB_LIB_RT)
    list(APPEND WB_BOOTMGR_LINK_DEPS "rt")
  endif()

  # Latest libcpp doesn't have one.
  if (WB_LIB_CPP_FS)
    list(APPEND WB_BOOTMGR_LINK_DEPS "c++fs")
  endif()
endif()

wb_cxx_shared_library(
  TARGET        ${WB_BOOTMGR_TARGET_NAME}
  VERSION       ${CMAKE_PROJECT_VERSION}
  DESCRIPTION   "Whitebox Boot Manager"
  SOURCE_DIR    ${WB_BOOTMGR_SOURCE_DIR}
  BINARY_DIR    ${WB_BOOTMGR_BINARY_DIR}
  CXX_DEFS      WB_BOOT_MANAGER_DLL=1
  LINK_OPTS     ${WB_BOOTMGR_LINK_OPTIONS}
  LINK_DEPS     ${WB_BOOTMGR_LINK_DEPS}
)
