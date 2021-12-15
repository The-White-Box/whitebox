# Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
# Use of this source code is governed by a 3-Clause BSD license that can be
# found in the LICENSE file.
#
# Base library project definition.

cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

set(WB_BASE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(WB_BASE_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(WB_BASE_TARGET_NAME "whitebox-base")

set(WB_BASE_LINK_OPTIONS "")
set(WB_BASE_LINK_DEPS
  absl::cleanup
  fmt
  g3log
  mimalloc)
set(WB_BASE_RUNTIME_DEPS
  g3log
  mimalloc)
set(WB_BASE_CXX_DEFINITIONS WB_BASE_DLL=1)

if (WB_OS_WIN)
  check_cxx_source_compiles("
    #include <windows.h>
    int main(int argc, char **argv) {
      PROCESS_MITIGATION_USER_SHADOW_STACK_POLICY new_policy;
      new_policy.EnableUserShadowStack = 1UL;
      return 0;
    }"
    WB_OS_WIN_HAS_PROCESS_MITIGATION_USER_SHADOW_STACK_POLICY)
  if (WB_OS_WIN_HAS_PROCESS_MITIGATION_USER_SHADOW_STACK_POLICY)
    list(APPEND WB_BASE_CXX_DEFINITIONS
      WB_OS_WIN_HAS_PROCESS_MITIGATION_USER_SHADOW_STACK_POLICY=1)
  endif()

  # Delay load DLLs we use in rare call paths.
  list(APPEND WB_BASE_LINK_OPTIONS
    "/DELAYLOAD:Avrt.dll")
  # Avrt for MMCSS support.
  # Delayimp for DLL delay load support.
  list(APPEND WB_BASE_LINK_DEPS Avrt Delayimp)
endif()

wb_cxx_shared_library(
  TARGET        ${WB_BASE_TARGET_NAME}
  VERSION       ${CMAKE_PROJECT_VERSION}
  DESCRIPTION   "Whitebox Base Abstractions and Definitions"
  SOURCE_DIR    ${WB_BASE_SOURCE_DIR}
  BINARY_DIR    ${WB_BASE_BINARY_DIR}
  CXX_DEFS      ${WB_BASE_CXX_DEFINITIONS}
  LINK_OPTS     ${WB_BASE_LINK_OPTIONS}
  LINK_DEPS     ${WB_BASE_LINK_DEPS}
  RUNTIME_DEPS  ${WB_BASE_RUNTIME_DEPS}
)

if (WB_BUILD_TESTS)
  set(WB_BASE_TESTS_LINK_DEPS
    # Should be first as needs redirect first.
    mimalloc
    absl::cleanup
    fmt
    g3log
    wb::whitebox-base)

  if (WB_OS_WIN)
    # _com_ptr_t support / winmm.
    list(APPEND WB_BASE_TESTS_LINK_DEPS "comsuppw;Winmm")
  endif()

  wb_cxx_test_exe_for_target(
    TARGET ${WB_BASE_TARGET_NAME}
    SOURCE_DIR ${WB_BASE_SOURCE_DIR}
    LINK_DEPS ${WB_BASE_TESTS_LINK_DEPS}
  )
endif()
