# Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
# Use of this source code is governed by a 3-Clause BSD license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.9 FATAL_ERROR)

# Compiler id for Apple Clang is now AppleClang.
if (POLICY CMP0025)
  cmake_policy(SET CMP0025 NEW)
endif (POLICY CMP0025)

# option() honor variables.
if (POLICY CMP0077)
  cmake_policy(SET CMP0077 NEW)
endif (POLICY CMP0077)

# target_link_libraries() allows use with targets in other directories.
if (POLICY CMP0079)
  cmake_policy(SET CMP0079 NEW)
endif (POLICY CMP0079)

# Binaries dir.
set(WB_BINARY_DIR "${CMAKE_BINARY_DIR}")
# Build system name.
set(WB_SYSTEM_NAME "${CMAKE_SYSTEM_NAME}")
# Is build OS MacOs.
set(WB_OS_MACOSX $<BOOL:WB_SYSTEM_NAME STREQUAL \"Darwin\">)
# Is build OS Linux.
set(WB_OS_LINUX $<BOOL:WB_SYSTEM_NAME STREQUAL \"Linux\">)
# Is build OS Windows.
set(WB_OS_WINDOWS $<BOOL:WB_SYSTEM_NAME STREQUAL \"Windows\">)
# Root dir.
set(WB_ROOT_DIR "${CMAKE_SOURCE_DIR}")
# Product name.
set(WB_PRODUCT_NAME "WhiteBox Games Collection")

# Includes.
if (CMAKE_MODULE_PATH)
  set(CMAKE_MODULE_PATH "${WB_ROOT_DIR}/cmake" "${CMAKE_MODULE_PATH}")
else()
  set(CMAKE_MODULE_PATH "${WB_ROOT_DIR}/cmake")
endif()

# Package information.
set(PACKAGE_NAME      "WhiteBox")
if (NOT DEFINED PACKAGE_VERSION)
  set(PACKAGE_VERSION_MAJOR 0)
  set(PACKAGE_VERSION_MINOR 1)
  set(PACKAGE_VERSION_PATCH 0)
  set(PACKAGE_VERSION_TWEAK 0)
  set(PACKAGE_VERSION   "${PACKAGE_VERSION_MAJOR}.${PACKAGE_VERSION_MINOR}.${PACKAGE_VERSION_PATCH}.${PACKAGE_VERSION_TWEAK}-dev")
endif()
set(PACKAGE_STRING    "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_TARNAME   "${PACKAGE_NAME}-${PACKAGE_VERSION}")
set(PACKAGE_BUGREPORT "https://github.com/dimhotepus/whitebox/issues")

project(${PACKAGE_NAME}
  VERSION ${PACKAGE_VERSION_MAJOR}.${PACKAGE_VERSION_MINOR}.${PACKAGE_VERSION_PATCH}.${PACKAGE_VERSION_TWEAK}
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(wb_functions)
wb_check_platform_requirements(${PACKAGE_NAME})

# Build type.
set(WB_BUILD_TYPE "${CMAKE_BUILD_TYPE}")
# Cxx compiler path.
set(WB_CXX_COMPILER "${CMAKE_CXX_COMPILER}")
# Configuration types.
set(WB_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}")
# Cxx compiler id.
set(WB_CXX_COMPILER_ID "${CMAKE_CXX_COMPILER_ID}")
# Cxx compiler frontend variant.
set(WB_CXX_COMPILER_FRONTEND_VARIANT "${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}")
# Export compile commands or not?
set(WB_EXPORT_COMPILE_COMMANDS ${CMAKE_EXPORT_COMPILE_COMMANDS})

## Find common dependencies.
# pthread
set(CMAKE_THREAD_PREFER_PTHREAD ON)
find_package(Threads REQUIRED)
set(WB_HAVE_PTHREAD "${CMAKE_USE_PTHREADS_INIT}")
list(APPEND CMAKE_REQUIRED_LIBRARIES Threads::Threads)

if (WB_CXX_COMPILER_ID MATCHES "Clang")
  # Clang or AppleClang.
  if (WB_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    message(STATUS "${PACKAGE_NAME} compiler Clang + MSVC frontend.")
    # Clang with clang-cl front end.
    include(compilers/wb_msvc)
  elseif (WB_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
    message(STATUS "${PACKAGE_NAME} compiler Clang + GNU frontend.")
    # Clang with regular front end.
    include(compilers/wb_clang)
  endif()
elseif (WB_CXX_COMPILER_ID STREQUAL "GNU")
  message(STATUS "${PACKAGE_NAME} compiler GNU.")
  include(compilers/wb_gcc)
elseif (WB_CXX_COMPILER_ID STREQUAL "MSVC")
  message(STATUS "${PACKAGE_NAME} compiler MSVC.")
  include(compilers/wb_msvc)
else()
  # Add compilers if needed.
  message(FATAL_ERROR "'${WB_CXX_COMPILER_ID}' cxx compiler is not supported. "
    "Please, define compiler/linker options for it.")
endif()

message(STATUS "${PACKAGE_NAME} cxx compiler: ${WB_CXX_COMPILER}.")
message(STATUS "${PACKAGE_NAME} build type: ${WB_BUILD_TYPE}.")
message(STATUS "${PACKAGE_NAME} configuration types: ${WB_CONFIGURATION_TYPES}.")

# Generate common product version info.
configure_file(
  ${WB_ROOT_DIR}/build/product_version_config.h.cmake.in
  ${WB_BINARY_DIR}/product_version_config.h
)

## Include sub-projects.
# Dependencies.
add_subdirectory("deps/g3log")

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# No need to install gtest.
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

# add_subdirectory("deps/googletest")
add_subdirectory("deps/mimalloc")

# Libraries.
add_subdirectory("base")
add_subdirectory("bootmgr")
add_subdirectory("whitebox-kernel")

# Apps.
add_subdirectory("apps/half-life-2")

# Tests.
enable_testing()

# Get all targets to add mimalloc allocators overrides.
wb_get_all_targets(WB_ALL_TARGETS "${WB_ROOT_DIR}")

# Ensure all dependencies reuse same memory allocators.
foreach(WB_TARGET_NAME ${WB_ALL_TARGETS})
  # Do not add mimalloc allocators to itself.
  if (NOT ${WB_TARGET_NAME} MATCHES "mimalloc")
    # Include the directories with the new files.
    target_include_directories(${WB_TARGET_NAME}
      PRIVATE ${WB_ROOT_DIR} "${WB_ROOT_DIR}/deps/mimalloc/include")

    # Add new / delete allocator overrides.
    target_sources(${WB_TARGET_NAME}
      PRIVATE
        ${WB_ROOT_DIR}/base/memory/memory_allocators_overrides.cc
    )

    # Add mimalloc to handle allocators.
    target_link_libraries(${WB_TARGET_NAME} mimalloc)

    # mimalloc should be copied by each target individually as haven't found way
    # to it here.
  endif()
endforeach()
