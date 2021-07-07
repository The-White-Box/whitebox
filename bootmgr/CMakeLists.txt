# Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
# Use of this source code is governed by a 3-Clause BSD license that can be
# found in the LICENSE file.
#
# Bootmgr library project definition.

cmake_minimum_required (VERSION 3.9 FATAL_ERROR)

set(WB_BOOTMGR_VERSION     ${CMAKE_PROJECT_VERSION})

project("wb::bootmgr"
  VERSION ${WB_BOOTMGR_VERSION}
  DESCRIPTION "Whitebox Boot Manager"
  LANGUAGES CXX)

set(WB_BOOTMGR_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(WB_BOOTMGR_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(WB_BOOTMGR_TARGET_NAME "bootmgr")
set(WB_CURRENT_TARGET_NAME ${WB_BOOTMGR_TARGET_NAME})
set(WB_CURRENT_TARGET_SUFFIX ".dll")

# We currently build app by finding all sources and header files.
# We then exclude specific files below.
wb_auto_sources(files "*.cc" "RECURSE" "${WB_BOOTMGR_SOURCE_DIR}")
wb_auto_sources(hfiles "*.h" "RECURSE" "${WB_BOOTMGR_SOURCE_DIR}")

# Generate project info.
configure_file(
  ${WB_ROOT_DIR}/build/app_version_config.h.cmake.in
  ${WB_BOOTMGR_BINARY_DIR}/gen/app_version_config.h
)

add_library(${WB_BOOTMGR_TARGET_NAME} SHARED ${files} ${hfiles})

# Include the directory with the new files.
target_include_directories(${WB_BOOTMGR_TARGET_NAME}
  PRIVATE
    ${WB_ROOT_DIR}
    ${WB_BOOTMGR_BINARY_DIR}/gen
    ${WB_BINARY_DIR}
)

# Specify project compile / link options.
wb_apply_compile_options_to_target(${WB_BOOTMGR_TARGET_NAME})

# Specify definitions for library.
target_compile_definitions(${WB_BOOTMGR_TARGET_NAME}
  PRIVATE WB_BOOTMGR_DLL=1
)

set(WB_BOOTMGR_DEPENDENT_LINK_LIBRARIES g3log mimalloc)
set(WB_BOOTMGR_OS_SPECIFIC_LINK_LIBRARIES "")

if (WB_OS_WINDOWS)
  set(WB_BOOTMGR_OS_SPECIFIC_LINK_LIBRARIES Winmm)
endif()

target_link_libraries(${WB_BOOTMGR_TARGET_NAME}
  ${WB_BOOTMGR_DEPENDENT_LINK_LIBRARIES}
  ${WB_BOOTMGR_OS_SPECIFIC_LINK_LIBRARIES}
)

# Specify library version / soversion.
set_target_properties(${WB_BOOTMGR_TARGET_NAME}
  PROPERTIES
    VERSION ${WB_BOOTMGR_VERSION}
    SOVERSION ${WB_BOOTMGR_VERSION}
)

# Append Windows specific.
if (WB_OS_WINDOWS)
  target_sources(${WB_BOOTMGR_TARGET_NAME}
    PRIVATE
      ${WB_ROOT_DIR}/build/windows/resource_scripts/windows_dll_base.rc
  )
endif()

# Add runtime dependencies.
wb_copy_all_target_dependencies_to_target_bin_dir(${WB_BOOTMGR_TARGET_NAME}
  "${WB_BOOTMGR_DEPENDENT_LINK_LIBRARIES}")
