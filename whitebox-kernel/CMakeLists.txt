# Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
# Use of this source code is governed by a 3-Clause BSD license that can be
# found in the LICENSE file.
#
# Bootmgr library project definition.

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

set(WB_WHITEBOX_KERNEL_VERSION     ${CMAKE_PROJECT_VERSION})

project("whitebox-kernel"
  VERSION ${WB_WHITEBOX_KERNEL_VERSION}
  DESCRIPTION "Whitebox Lightweight Kernel"
  LANGUAGES CXX)

set(WB_WHITEBOX_KERNEL_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(WB_WHITEBOX_KERNEL_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(WB_WHITEBOX_KERNEL_TARGET_NAME "whitebox-kernel")
set(WB_CURRENT_TARGET_NAME ${WB_WHITEBOX_KERNEL_TARGET_NAME})
set(WB_CURRENT_TARGET_SUFFIX ".dll")

# We currently build app by finding all sources and header files.
# We then exclude specific files below.
wb_auto_sources(files "*.cc" "RECURSE" "${WB_WHITEBOX_KERNEL_SOURCE_DIR}")
wb_auto_sources(hfiles "*.h" "RECURSE" "${WB_WHITEBOX_KERNEL_SOURCE_DIR}")

if (NOT WB_OS_WIN)
  wb_remove_matches_from_lists(files hfiles
    MATCHES
      "^${WB_WHITEBOX_KERNEL_SOURCE_DIR}/main_window.h"
      "^${WB_WHITEBOX_KERNEL_SOURCE_DIR}/main_window.cc"
  )
endif()

# Generate project info.
configure_file(
  ${WB_ROOT_DIR}/build/app_version_config.h.cmake.in
  ${WB_WHITEBOX_KERNEL_BINARY_DIR}/gen/app_version_config.h
)

add_library(${WB_WHITEBOX_KERNEL_TARGET_NAME} SHARED ${files} ${hfiles})

# Include the directory with the new files.
target_include_directories(${WB_WHITEBOX_KERNEL_TARGET_NAME}
  PRIVATE
    ${WB_ROOT_DIR}
    ${WB_WHITEBOX_KERNEL_BINARY_DIR}/gen
    ${WB_BINARY_DIR}
)

# Specify project compile / link options.
wb_apply_compile_options_to_target(${WB_WHITEBOX_KERNEL_TARGET_NAME})

# Specify definitions for library.
target_compile_definitions(${WB_WHITEBOX_KERNEL_TARGET_NAME}
  PRIVATE WB_WHITEBOX_KERNEL_DLL=1
)

set(WB_WHITEBOX_KERNEL_DEPENDENT_LINK_LIBRARIES base g3log mimalloc)

target_link_libraries(${WB_WHITEBOX_KERNEL_TARGET_NAME} PRIVATE base g3log mimalloc)

# Specify library version / soversion.
set_target_properties(${WB_WHITEBOX_KERNEL_TARGET_NAME}
  PROPERTIES
    VERSION ${WB_WHITEBOX_KERNEL_VERSION}
    SOVERSION ${WB_WHITEBOX_KERNEL_VERSION}
)

# Append Windows specific.
if (WB_OS_WIN)
  target_sources(${WB_WHITEBOX_KERNEL_TARGET_NAME}
    PRIVATE
      ${WB_ROOT_DIR}/build/windows/resource_scripts/windows_dll_base.rc
  )
endif()

set(WB_WHITEBOX_KERNEL_DEPENDENCIES mimalloc)

# Add runtime dependencies.
wb_copy_all_target_dependencies_to_target_bin_dir(${WB_WHITEBOX_KERNEL_TARGET_NAME}
  "${WB_WHITEBOX_KERNEL_DEPENDENCIES}")
