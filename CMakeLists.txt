# Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
# Use of this source code is governed by a 3-Clause BSD license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

# Includes.
if (CMAKE_MODULE_PATH)
  set(CMAKE_MODULE_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
    "${CMAKE_MODULE_PATH}")
else()
  set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
endif()

# Package information.
set(PACKAGE_NAME      "WhiteBox")
if (NOT DEFINED PACKAGE_VERSION)
  set(PACKAGE_VERSION_MAJOR 0)
  set(PACKAGE_VERSION_MINOR 1)
  set(PACKAGE_VERSION_PATCH 0)
  set(PACKAGE_VERSION   "${PACKAGE_VERSION_MAJOR}.${PACKAGE_VERSION_MINOR}.${PACKAGE_VERSION_PATCH}-dev")
endif()
set(PACKAGE_STRING    "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_TARNAME   "${PACKAGE_NAME}-${PACKAGE_VERSION}")
set(PACKAGE_BUGREPORT "https://github.com/dimhotepus/whitebox/issues")

project(${PACKAGE_NAME}
  VERSION ${PACKAGE_VERSION_MAJOR}.${PACKAGE_VERSION_MINOR}.${PACKAGE_VERSION_PATCH}
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(whitebox_functions)
verify_target_architecture(${PACKAGE_NAME})

# Need Pthread.
set(CMAKE_THREAD_PREFER_PTHREAD ON)
find_package(Threads REQUIRED)
set(WHITEBOX_HAVE_PTHREAD "${CMAKE_USE_PTHREADS_INIT}")
list(APPEND CMAKE_REQUIRED_LIBRARIES Threads::Threads)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  # Clang or AppleClang.
  if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    # Clang with clang-cl front end.
    include(compilers/whitebox_msvc)
  elseif (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
    # Clang with regular front end.
    include(compilers/whitebox_clang)
  endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  include(compilers/whitebox_gcc)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # MSVC.
  include(compilers/whitebox_msvc)
else()
  # Add compilers if needed.
  message(FATAL_ERROR "'${CMAKE_CXX_COMPILER_ID}' compiler is not supported. "
    "Please, define compiler/linker options for it.")
endif()

include(whitebox_config_checks)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/whitebox-config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/whitebox-config.h
)

set(WHITEBOX_ROOT_PATH ${CMAKE_SOURCE_DIR})

# Include sub-projects.
add_subdirectory("apps/half-life-2")
