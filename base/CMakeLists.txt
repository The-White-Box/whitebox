# Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
# Use of this source code is governed by a 3-Clause BSD license that can be
# found in the LICENSE file.
#
# Base library project definition.

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

set(WB_BASE_VERSION     ${CMAKE_PROJECT_VERSION})

project("base"
  VERSION ${WB_BASE_VERSION}
  DESCRIPTION "Whitebox Base Abstractions and Definitions"
  LANGUAGES CXX)

set(WB_BASE_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(WB_BASE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(WB_BASE_TARGET_NAME "base")
set(WB_CURRENT_TARGET_NAME ${WB_BASE_TARGET_NAME})
set(WB_CURRENT_TARGET_SUFFIX ".dll")

# We currently build app by finding all sources and header files.
# We then exclude specific files below.
wb_auto_sources(files "*.cc" "RECURSE" "${WB_BASE_SOURCE_DIR}")
wb_auto_sources(hfiles "*.h" "RECURSE" "${WB_BASE_SOURCE_DIR}")

wb_remove_matches_from_lists(files hfiles
  MATCHES
    "^${WB_BASE_SOURCE_DIR}(.*)?_tests.cc$"
)

if (NOT WB_OS_WIN)
  wb_remove_matches_from_lists(files hfiles
    MATCHES
      "^${WB_BASE_SOURCE_DIR}/windows/"
  )
endif()

# Generate project info.
configure_file(
  ${WB_ROOT_DIR}/build/app_version_config.h.cmake.in
  ${WB_BASE_BINARY_DIR}/gen/app_version_config.h
)

add_library(${WB_BASE_TARGET_NAME} SHARED ${files} ${hfiles})

# Include the directories with the new files.
target_include_directories(${WB_BASE_TARGET_NAME}
  PRIVATE
    ${WB_ROOT_DIR}
    ${WB_BASE_BINARY_DIR}/gen
    ${WB_BINARY_DIR}
)

# Specify project compile / link options.
wb_apply_compile_options_to_target(${WB_BASE_TARGET_NAME})

# Specify definitions for library.
target_compile_definitions(${WB_BASE_TARGET_NAME}
  PRIVATE WB_BASE_DLL=1
)

set(WB_BASE_LINK_LIBRARIES g3log mimalloc)

if (WB_OS_WIN)
  # Delayimp for delayed DLL loading.
  # Comctl32 for TaskDialogIndirect.
  # Shell32 for ShellExecuteW.
  list(APPEND WB_BASE_LINK_LIBRARIES "Delayimp;Comctl32;Shell32")

  # Do not use Comctl32 & Shell32 by default, so speed up initial app load.
  target_link_options(${WB_BASE_TARGET_NAME}
    PRIVATE
      /DELAYLOAD:Comctl32.dll /DELAYLOAD:Shell32.dll)
endif()

target_link_libraries(${WB_BASE_TARGET_NAME} PRIVATE ${WB_BASE_LINK_LIBRARIES})

# Specify library version / soversion.
set_target_properties(${WB_BASE_TARGET_NAME}
  PROPERTIES
    VERSION ${WB_BASE_VERSION}
    SOVERSION ${WB_BASE_VERSION}
)

# Append Windows specific.
if (WB_OS_WIN)
  target_sources(${WB_BASE_TARGET_NAME}
    PRIVATE
      ${WB_ROOT_DIR}/build/windows/resource_scripts/windows_dll_base.rc
  )
endif()

set(WB_BASE_DEPENDENCIES g3log mimalloc)

# Add runtime dependencies.
wb_copy_all_target_dependencies_to_target_bin_dir(${WB_BASE_TARGET_NAME} "${WB_BASE_DEPENDENCIES}")

if (WB_BUILD_TESTS)
  # We currently build tests by finding all sources and header files.
  # We then exclude specific files below.
  wb_auto_sources(tests_files "*_tests.cc" "RECURSE" "${WB_BASE_SOURCE_DIR}")

  if (NOT WB_OS_WIN)
    wb_remove_matches_from_lists(tests_files
        MATCHES
        "^${WB_BASE_SOURCE_DIR}/windows/"
        )
  endif ()

  add_executable("${WB_BASE_TARGET_NAME}_tests" ${tests_files})

  # Specify project compile / link options.
  wb_apply_compile_options_to_target("${WB_BASE_TARGET_NAME}_tests")

  # Consume GTest as shared library to reduce tests size (slow down them a bit).
  target_compile_definitions("${WB_BASE_TARGET_NAME}_tests" PRIVATE GTEST_LINKED_AS_SHARED_LIBRARY=1)
  target_link_libraries("${WB_BASE_TARGET_NAME}_tests" PRIVATE GTest::gtest_main)

  include(GoogleTest)
  gtest_discover_tests("${WB_BASE_TARGET_NAME}_tests")
endif()
