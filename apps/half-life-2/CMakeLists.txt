# Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
# Use of this source code is governed by a 3-Clause BSD license that can be
# found in the LICENSE file.
#
# Half-Life 2 launcher project definition.

cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

set(WB_HALF_LIFE2_SOURCE_DIR   ${CMAKE_CURRENT_SOURCE_DIR})
set(WB_HALF_LIFE2_BINARY_DIR   ${CMAKE_CURRENT_BINARY_DIR})
set(WB_HALF_LIFE2_TARGET_NAME  "half-life-2")

set(WB_HALF_LIFE2_LINK_OPTIONS "")

set(WB_HALF_LIFE2_LINK_DEPS
  # Should be first as linker requires it.
  mimalloc
  absl::flags
  absl::flags_parse
  absl::flags_usage
  absl::strings
  fmt
  g3log
  wb::whitebox-base
  wb::whitebox-cpu-driver
  wb::whitebox-ui)
if (WB_OS_WIN)
  # _com_ptr_t support / winmm.
  list(APPEND WB_HALF_LIFE2_LINK_DEPS mimalloc-redirect comsuppw Winmm)
elseif (WB_OS_LINUX)
  # dlopen, filesystem.
  list(APPEND WB_HALF_LIFE2_LINK_DEPS "dl;stdc++fs")
elseif (WB_OS_MACOS)
  # dlopen, filesystem.
  list(APPEND WB_HALF_LIFE2_LINK_DEPS "dl;c++fs")
endif()

set(WB_HALF_LIFE2_RUNTIME_DEPS
  # Should be first as linker requires it.
  mimalloc
  mimalloc-redirect
  marl
  g3log
  wb::whitebox-base
  wb::whitebox-boot-manager
  wb::whitebox-cpu-driver
  wb::whitebox-hid-driver
  wb::whitebox-kernel
  wb::whitebox-ui)
if (NOT WB_OS_WIN)
  list(APPEND WB_HALF_LIFE2_LINK_DEPS
    "SDL3::SDL3-shared"
    "SDL3_image::SDL3_image")
  list(APPEND WB_HALF_LIFE2_RUNTIME_DEPS
    "SDL3::SDL3-shared"
    "SDL3_image::SDL3_image")
endif()

if (WB_OS_WIN)
  # Delay load DLLs we use in rare call paths.
  list(APPEND WB_HALF_LIFE2_LINK_OPTIONS "/DELAYLOAD:Dbghelp.dll")
  # Dbghelp for abseil related sym stuff.
  # Delayimp for DLL delay load support.
  list(APPEND WB_HALF_LIFE2_LINK_DEPS Dbghelp Delayimp)
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

target_sources(${WB_HALF_LIFE2_TARGET_NAME}
  PRIVATE
    ${WB_ROOT_DIR}/apps/base_flags.cc
    ${WB_ROOT_DIR}/apps/base_flags.h
    ${WB_ROOT_DIR}/apps/boot_heap_allocator.cc
    ${WB_ROOT_DIR}/apps/boot_heap_allocator.h
    ${WB_ROOT_DIR}/apps/cpu_feature_checks.cc
    ${WB_ROOT_DIR}/apps/cpu_feature_checks.h
    ${WB_ROOT_DIR}/apps/i18n_creator.cc
    ${WB_ROOT_DIR}/apps/i18n_creator.h
    ${WB_ROOT_DIR}/apps/parse_command_line.cc
    ${WB_ROOT_DIR}/apps/parse_command_line.h
)

if (WB_OS_WIN)
  # Command line args.
  target_sources(${WB_HALF_LIFE2_TARGET_NAME}
    PRIVATE
      ${WB_ROOT_DIR}/apps/args_win.cc
      ${WB_ROOT_DIR}/apps/args_win.h
  )
endif()

if (NOT WB_OS_WIN)
  wb_copy_target_asset_to_target_bin_dir(${WB_HALF_LIFE2_TARGET_NAME}
    "images/Half-Life 2 icon.png")
endif()

wb_copy_licenses_to_target_bin_dir(${WB_HALF_LIFE2_TARGET_NAME})
