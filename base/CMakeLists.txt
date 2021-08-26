# Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
# Use of this source code is governed by a 3-Clause BSD license that can be
# found in the LICENSE file.
#
# Base library project definition.

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

set(WB_BASE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(WB_BASE_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(WB_BASE_TARGET_NAME "base")

set(WB_BASE_LINK_OPTIONS "")
set(WB_BASE_LINK_DEPS g3log mimalloc)
set(WB_BASE_RUNTIME_DEPS g3log mimalloc)

if (WB_OS_WIN)
  # Delay load DLLs we use in rare call paths.
  list(APPEND WB_BASE_LINK_OPTIONS
    "/DELAYLOAD:Comctl32.dll" "/DELAYLOAD:Shell32.dll")
  # Delayimp for DLL dealy load support.
  # Comctl32 for TaskDialogIndirect.
  # Shell32 for ShellExecuteW.
  list(APPEND WB_BASE_LINK_DEPS Delayimp Comctl32 Shell32)
endif()

wb_cxx_shared_library(
  TARGET        ${WB_BASE_TARGET_NAME}
  VERSION       ${CMAKE_PROJECT_VERSION}
  DESCRIPTION   "Whitebox Base Abstractions and Definitions"
  SOURCE_DIR    ${WB_BASE_SOURCE_DIR}
  BINARY_DIR    ${WB_BASE_BINARY_DIR}
  CXX_DEFS      WB_BASE_DLL=1
  LINK_OPTS     ${WB_BASE_LINK_OPTIONS}
  LINK_DEPS     ${WB_BASE_LINK_DEPS}
  RUNTIME_DEPS  ${WB_BASE_RUNTIME_DEPS}
)

if (WB_BUILD_TESTS)
  set(WB_BASE_TESTS_LINK_DEPS base g3log mimalloc)

  if (WB_OS_WIN)
    # _com_ptr_t support.
    list(APPEND WB_BASE_TESTS_LINK_DEPS "comsuppw")
  endif()

  wb_cxx_test_exe_for_target(
      TARGET ${WB_BASE_TARGET_NAME}
      SOURCE_DIR ${WB_BASE_SOURCE_DIR}
      LINK_DEPS ${WB_BASE_TESTS_LINK_DEPS}
  )
endif()
