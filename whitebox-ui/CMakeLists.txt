# Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
# Use of this source code is governed by a 3-Clause BSD license that can be
# found in the LICENSE file.
#
# UI library project definition.

cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

set(WB_WHITEBOX_UI_SOURCE_DIR   ${CMAKE_CURRENT_SOURCE_DIR})
set(WB_WHITEBOX_UI_BINARY_DIR   ${CMAKE_CURRENT_BINARY_DIR})
set(WB_WHITEBOX_UI_TARGET_NAME  "whitebox-ui")

set(WB_WHITEBOX_UI_LINK_OPTIONS "")
set(WB_WHITEBOX_UI_LINK_DEPS
  fmt
  g3log
  mimalloc
  wb::whitebox-base)
if (NOT WB_OS_WIN)
  list(APPEND WB_WHITEBOX_UI_LINK_DEPS "SDL2::SDL2" "SDL2::image")
endif()

set(WB_WHITEBOX_UI_RUNTIME_DEPS
  g3log
  mimalloc
  wb::whitebox-base)
if (NOT WB_OS_WIN)
  list(APPEND WB_WHITEBOX_UI_RUNTIME_DEPS "SDL2::SDL2" "SDL2::image")
else()
  # Delay load DLLs we use in rare call paths.
  list(APPEND WB_WHITEBOX_UI_LINK_OPTIONS
    "/DELAYLOAD:Comctl32.dll" "/DELAYLOAD:Shell32.dll")
  # Comctl32 for TaskDialogIndirect.
  # Delayimp for DLL delay load support.
  # Shell32 for ShellExecuteW.
  list(APPEND WB_WHITEBOX_UI_LINK_DEPS Comctl32 Delayimp Shell32)
endif()

wb_cxx_shared_library(
  TARGET        ${WB_WHITEBOX_UI_TARGET_NAME}
  VERSION       ${CMAKE_PROJECT_VERSION}
  DESCRIPTION   "Whitebox cross-platform UI primitives"
  SOURCE_DIR    ${WB_WHITEBOX_UI_SOURCE_DIR}
  BINARY_DIR    ${WB_WHITEBOX_UI_BINARY_DIR}
  CXX_DEFS      WB_WHITEBOX_UI_DLL=1
  LINK_OPTS     ${WB_WHITEBOX_UI_LINK_OPTIONS}
  LINK_DEPS     ${WB_WHITEBOX_UI_LINK_DEPS}
  RUNTIME_DEPS  ${WB_WHITEBOX_UI_RUNTIME_DEPS}
)
