# Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
# Use of this source code is governed by a 3-Clause BSD license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

option(WB_BUILD_TESTS "Build tests or not." OFF)

# Compiler id for Apple Clang is now AppleClang.
if (POLICY CMP0025)
  cmake_policy(SET CMP0025 NEW)
endif()

# option() honor variables.
if (POLICY CMP0077)
  cmake_policy(SET CMP0077 NEW)
endif()

# target_link_libraries() allows use with targets in other directories.
if (POLICY CMP0079)
  cmake_policy(SET CMP0079 NEW)
endif()

set(WB_ROOT_DIR "${CMAKE_SOURCE_DIR}" CACHE STRING
  "Directory root of the project.")

# Includes.
if (CMAKE_MODULE_PATH)
  set(CMAKE_MODULE_PATH "${WB_ROOT_DIR}/cmake" "${CMAKE_MODULE_PATH}")
else()
  set(CMAKE_MODULE_PATH "${WB_ROOT_DIR}/cmake")
endif()

# Package information.
set(PACKAGE_NAME      "WhiteBox")
if (NOT DEFINED PACKAGE_VERSION)
  set(PACKAGE_VER_MAJOR 0)
  set(PACKAGE_VER_MAJOR 1)
  set(PACKAGE_VER_PATCH 0)
  set(PACKAGE_VER_TWEAK 0)
  set(PACKAGE_VERSION
    "${PACKAGE_VER_MAJOR}.${PACKAGE_VER_MAJOR}.${PACKAGE_VER_PATCH}.${PACKAGE_VER_TWEAK}-dev")
endif()
set(PACKAGE_STRING    "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_TARNAME   "${PACKAGE_NAME}-${PACKAGE_VERSION}")
set(PACKAGE_BUGREPORT "https://github.com/The-White-Box/whitebox/issues")

project(${PACKAGE_NAME}
  VERSION ${PACKAGE_VER_MAJOR}.${PACKAGE_VER_MAJOR}.${PACKAGE_VER_PATCH}.${PACKAGE_VER_TWEAK}
  LANGUAGES CXX)

message(STATUS "[common]: CMake version: ${CMAKE_VERSION}.")
message(STATUS "[common]: ${PACKAGE_NAME} version: ${PACKAGE_VERSION}.")

set(WB_BINARY_DIR "${CMAKE_BINARY_DIR}" CACHE STRING
  "Directory of the project binaries output.")
set(WB_SYSTEM_NAME "${CMAKE_SYSTEM_NAME}" CACHE STRING
  "Name of the build system.")

if (${WB_SYSTEM_NAME} STREQUAL "Darwin")
  set(WB_OS_MACOSX ON CACHE BOOL "MacOS is build system.")
else()
  set(WB_OS_MACOSX OFF CACHE BOOL "MacOS is NOT build system.")
endif()

if (${WB_SYSTEM_NAME} STREQUAL "Linux")
  set(WB_OS_LINUX ON CACHE BOOL "Linux distro is build system.")
else()
  set(WB_OS_LINUX OFF CACHE BOOL "Linux distro is NOT build system.")
endif()

if (${WB_SYSTEM_NAME} STREQUAL "Windows")
  set(WB_OS_WIN ON CACHE BOOL "Windows is build system.")
else()
  set(WB_OS_WIN OFF CACHE BOOL "Windows is NOT build system.")
endif()

set(WB_PRODUCT_NAME "WhiteBox Games Collection" CACHE STRING
  "Name of the product.")

set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(wb_common_functions)
include(wb_build_functions)
wb_check_platform_requirements(${PACKAGE_NAME})

set(WB_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING
  "CMake build type used to build ${PACKAGE_NAME}.")
set(WB_CXX_COMPILER "${CMAKE_CXX_COMPILER}" CACHE STRING
  "Compiler used to build ${PACKAGE_NAME}.")
set(WB_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING
  "CMake available build types for building ${PACKAGE_NAME}.")
set(WB_CXX_COMPILER_ID "${CMAKE_CXX_COMPILER_ID}" CACHE STRING
  "Compiler id used to build ${PACKAGE_NAME}.")
set(WB_CXX_COMPILER_FRONTEND_VARIANT "${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}"
  CACHE STRING "Compiler frontend used to build ${PACKAGE_NAME}.")
set(WB_EXPORT_COMPILE_COMMANDS ${CMAKE_EXPORT_COMPILE_COMMANDS} CACHE BOOL
  "Enable/Disable output of compile commands during generation.")

if (WB_CXX_COMPILER_ID MATCHES "Clang")
  set(WB_COMPILER_CLANG ON CACHE BOOL "Compiler is Clang.")
else()
  set(WB_COMPILER_CLANG OFF CACHE BOOL "Compiler is NOT Clang.")
endif()

if (WB_CXX_COMPILER_ID STREQUAL "GNU")
  set(WB_COMPILER_GCC ON CACHE BOOL "Compiler is GCC.")
else()
  set(WB_COMPILER_GCC OFF CACHE BOOL "Compiler is NOT GCC.")
endif()

if (WB_CXX_COMPILER_ID STREQUAL "MSVC")
  set(WB_COMPILER_MSVC ON CACHE BOOL "Compiler is MSVC.")
else()
  set(WB_COMPILER_MSVC OFF CACHE BOOL "Compiler is NOT MSVC.")
endif()

## Find common dependencies.

# Git.
find_package(Git)
if (Git_FOUND)
  message(STATUS
    "[common]: Git version ${GIT_VERSION_STRING} found at '${GIT_EXECUTABLE}'.")

  # Get the current working branch.
  execute_process(
    COMMAND "${GIT_EXECUTABLE}" rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${WB_ROOT_DIR}
    OUTPUT_VARIABLE WB_GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  # Get the latest commit hash.
  execute_process(
    COMMAND "${GIT_EXECUTABLE}" rev-parse HEAD
    WORKING_DIRECTORY ${WB_ROOT_DIR}
    OUTPUT_VARIABLE WB_GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  # Get the subject of the commit.
  execute_process(
    COMMAND "${GIT_EXECUTABLE}" log -1 --format=%s
    WORKING_DIRECTORY "${WB_ROOT_DIR}"
    OUTPUT_VARIABLE WB_GIT_COMMIT_SUBJECT
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

  message(STATUS
    "[common]: Git using branch '${WB_GIT_BRANCH}', "
    "commit ${WB_GIT_COMMIT_HASH} | ${WB_GIT_COMMIT_SUBJECT}.")
endif()

# pthread.
set(CMAKE_THREAD_PREFER_PTHREAD ON)
find_package(Threads REQUIRED)
set(WB_HAVE_PTHREAD "${CMAKE_USE_PTHREADS_INIT}" CACHE BOOL
  "Do build host has pthreads library?")
list(APPEND CMAKE_REQUIRED_LIBRARIES Threads::Threads)

if (XCODE_VERSION)
  message(STATUS
    "[common]: ${PACKAGE_NAME} using XCode version: ${XCODE_VERSION}.")
endif()

if (WB_COMPILER_CLANG)
  # Clang or AppleClang.
  if (WB_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    message(STATUS
      "[common]: ${PACKAGE_NAME} use compiler Clang + MSVC frontend.")

    # Clang with clang-cl front end.
    include(compilers/wb_msvc)
  elseif (WB_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
    message(STATUS
      "[common]: ${PACKAGE_NAME} use compiler Clang + GNU frontend.")

    # Clang with regular front end.
    include(compilers/wb_clang)
  elseif (NOT WB_CXX_COMPILER_FRONTEND_VARIANT)
    # No compiler frontend means Apple one.
    message(STATUS
      "[common]: ${PACKAGE_NAME} use compiler Clang + Apple frontend.")

    # Clang with regular front end.
    include(compilers/wb_clang)
  else()
    message(FATAL_ERROR
      "[common]: Using Clang with unknown frontend "
      "'${WB_CXX_COMPILER_FRONTEND_VARIANT}'. "
      "Please, define compiler/linker options for it.")
  endif()
elseif (WB_COMPILER_GCC)
  message(STATUS "[common]: ${PACKAGE_NAME} use compiler GNU.")

  include(compilers/wb_gcc)
elseif (WB_COMPILER_MSVC)
  message(STATUS "[common]: ${PACKAGE_NAME} use compiler MSVC.")
  message(STATUS
    "[common]: ${PACKAGE_NAME} MSVC         version: ${MSVC_VERSION}")
  message(STATUS
    "[common]: ${PACKAGE_NAME} MSVC toolset version: ${MSVC_TOOLSET_VERSION}")

  include(compilers/wb_msvc)
else()
  # Add compilers if needed.
  message(FATAL_ERROR 
    "[common]: '${WB_CXX_COMPILER_ID}' cxx compiler is not supported. "
    "Please, define compiler/linker options for it.")
endif()

message(STATUS "[common]: ${PACKAGE_NAME} cxx compiler: ${WB_CXX_COMPILER}.")
message(STATUS "[common]: ${PACKAGE_NAME} build type: ${WB_BUILD_TYPE}.")
message(STATUS
  "[common]: ${PACKAGE_NAME} configuration types: ${WB_CONFIGURATION_TYPES}.")

# Generate common product version info.
configure_file(
  ${WB_ROOT_DIR}/build/product_version_config.h.cmake.in
  ${WB_BINARY_DIR}/product_version_config.h
)

## Include sub-projects.
# Dependencies.
add_subdirectory("deps/g3log")

if (WB_BUILD_TESTS)
  # Prevent overriding the parent project's compiler/linker settings.
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

  add_subdirectory("deps/googletest")

  # Disallow macro name clamps.
  target_compile_definitions(gtest
    PUBLIC
      GTEST_DONT_DEFINE_FAIL=1
      GTEST_DONT_DEFINE_SUCCEED=1
      GTEST_DONT_DEFINE_TEST=1
  )

  if (WB_COMPILER_MSVC)
    # Prevent MSVC linker warning as error for __except + /guard:ehcont.
    target_compile_options(gtest PRIVATE /guard:ehcont)
    target_link_options(gtest PRIVATE /guard:ehcont)
  elseif (WB_COMPILER_CLANG OR WB_COMPILER_GCC)
    # Prevent linker warning "reloc which may overflow at runtime" (-fPIC).
    target_compile_options(gtest PRIVATE -fPIC)
  endif()
endif()

add_subdirectory("deps/mimalloc")

# Libraries.
add_subdirectory("base")
add_subdirectory("bootmgr")

# Hardware Abstraction Layer.
add_subdirectory("hal/drivers/hid")

# Kernel.
add_subdirectory("whitebox-kernel")

# Apps.
add_subdirectory("apps/half-life-2")

if (WB_BUILD_TESTS)
  enable_testing()
endif()

# Get all targets to add mimalloc allocators overrides.
wb_get_all_targets(WB_ALL_TARGETS "${WB_ROOT_DIR}")

# Ensure all dependencies reuse same memory allocators.
foreach(WB_TARGET_NAME ${WB_ALL_TARGETS})
  # Do not add mimalloc / custom targets allocators to itself and some specific
  # actions.
  if (NOT ${WB_TARGET_NAME} MATCHES "mimalloc" AND
      NOT ${WB_TARGET_NAME} MATCHES "clean-cmake")
    # Include the directories with the new files.
    target_include_directories(${WB_TARGET_NAME}
      PRIVATE ${WB_ROOT_DIR} "${WB_ROOT_DIR}/deps/mimalloc/include")

    # Add new / delete allocator overrides.
    target_sources(${WB_TARGET_NAME}
      PRIVATE
        ${WB_ROOT_DIR}/base/memory/memory_allocators_overrides.cc
    )

    # Add mimalloc to handle allocators.
    target_link_libraries(${WB_TARGET_NAME} PRIVATE mimalloc)

    # mimalloc should be copied by each target individually as haven't found way
    # to it here.
  endif()
endforeach()
