# Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
# Use of this source code is governed by a 3-Clause BSD license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

# Controls either WhiteBox tests be build or not.
option(WB_BUILD_TESTS "Build WhiteBox tests or not." OFF)

if (WB_BUILD_TESTS)
  enable_testing()
endif()

# Compiler id for Apple Clang is now AppleClang.
if (POLICY CMP0025)
  cmake_policy(SET CMP0025 NEW)
endif()

# Include file check macros honor CMAKE_REQUIRED_LIBRARIES.
if (POLICY CMP0075)
  cmake_policy(SET CMP0075 NEW)
endif()

# option() honor variables.
if (POLICY CMP0077)
  cmake_policy(SET CMP0077 NEW)
endif()

# target_link_libraries() allows use with targets in other directories.
if (POLICY CMP0079)
  cmake_policy(SET CMP0079 NEW)
endif()

# Setup root directory of the WhiteBox project.
set(WB_ROOT_DIR "${CMAKE_SOURCE_DIR}" CACHE STRING
  "Directory root of the WhiteBox project.")

# CMake should find its new includes.
if (CMAKE_MODULE_PATH)
  set(CMAKE_MODULE_PATH "${WB_ROOT_DIR}/cmake" "${CMAKE_MODULE_PATH}")
else()
  set(CMAKE_MODULE_PATH "${WB_ROOT_DIR}/cmake")
endif()

## Package information.
set(PACKAGE_NAME   "WhiteBox")

# Git is required to build software version via Git.
find_package(Git)

if (NOT DEFINED PACKAGE_VERSION)
  if (NOT Git_FOUND)
    message(FATAL "[common]: Unable to extract Git software version - no Git.")
  endif()

  message(STATUS "[common]: Extracting Git software version.")

  if ("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Windows")
    execute_process(COMMAND cmd /c "git rev-list --branches HEAD | find /v " " /c" OUTPUT_VARIABLE WB_GIT_VERSION WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  else()
    execute_process(COMMAND bash "-c" "git rev-list --branches HEAD | wc -l | tr -d ' ' | tr -d '\n'" OUTPUT_VARIABLE WB_GIT_VERSION WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  endif()

  set(PACKAGE_VER_PATCH 0)
  math(EXPR PACKAGE_VER_TWEAK ${WB_GIT_VERSION})
  message(STATUS
    "[common]: Build version from Git (patch.tweak): ${PACKAGE_VER_PATCH}.${PACKAGE_VER_TWEAK}.")

  set(PACKAGE_VER_MAJOR 1)
  set(PACKAGE_VER_MINOR 0)

  set(PACKAGE_VERSION
    "${PACKAGE_VER_MAJOR}.${PACKAGE_VER_MINOR}.${PACKAGE_VER_PATCH}.${PACKAGE_VER_TWEAK}")
endif()
set(PACKAGE_STRING     "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_TARNAME    "${PACKAGE_NAME}-${PACKAGE_VERSION}")
set(PACKAGE_BUGREPORT  "https://github.com/The-White-Box/whitebox/issues")

project(${PACKAGE_NAME}
  VERSION ${PACKAGE_VER_MAJOR}.${PACKAGE_VER_MINOR}.${PACKAGE_VER_PATCH}.${PACKAGE_VER_TWEAK}
  LANGUAGES CXX)

message(STATUS "[common]: CMake version: ${CMAKE_VERSION}.")
message(STATUS "[common]: ${PACKAGE_NAME} version: ${PACKAGE_VERSION}.")

## Common variables.

set(WB_BINARY_DIR "${CMAKE_BINARY_DIR}" CACHE STRING
  "Directory of the WhiteBox binaries output.")
set(WB_SYSTEM_NAME "${CMAKE_SYSTEM_NAME}" CACHE STRING
  "Name of the WhiteBox build system.")

if (${WB_SYSTEM_NAME} STREQUAL "Darwin")
  set(WB_OS_MACOS ON CACHE BOOL "MacOS is build system.")
else()
  set(WB_OS_MACOS OFF CACHE BOOL "MacOS is NOT build system.")
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

if (WB_OS_MACOS OR WB_OS_LINUX)
  set(WB_OS_POSIX ON CACHE BOOL "POSIX compatible os is build system.")
else()
  set(WB_OS_POSIX OFF CACHE BOOL "POSIX compatible os is NOT build system.")
endif()

set(WB_PRODUCT_NAME "WhiteBox Games Collection" CACHE STRING
  "Name of the WhiteBox product.")

# Minimum standard level is C++20.
set(CMAKE_CXX_STANDARD 20)
# On g++ this ensures: -std=c++20 and not -std=gnu++20
set(CMAKE_CXX_EXTENSIONS OFF)
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

## Common dependencies.

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

## Compiler / linker options.

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

## Compiler features detection.

wb_check_cxx_compiles("
  #include <iostream>
  int main(int argc, char **argv) {
    if (argc > 1) [[likely]] {
      std::cout << \"argc > 1\";
    }

    if (argv == nullptr) [[unlikely]] {
      std::cout << \"argv == nullptr\";
    }

    return 0;
  }"
  WB_CPP_HAS_LIKELY_UNLIKELY_ATTRIBUTES "[[likely|unlikely]]")

# Generate common product version info.
configure_file(
  ${WB_ROOT_DIR}/build/product_version_config.h.cmake.in
  ${WB_BINARY_DIR}/product_version_config.h
)

if (WB_OS_WIN)
  # Remove clashing min/max macroses from windows headers.
  add_definitions(-DNOMINMAX)
endif()

## Product dependencies.

# fmt.
add_subdirectory("deps/fmt")
if (WB_COMPILER_MSVC)
  # Prevent MSVC warning C4530: C++ exception handler used, but unwind
  # semantics are not enabled.
  target_compile_options(
    fmt
    PRIVATE
      /EH${WB_MSVC_EXCEPTION_HANDLING_MODEL}
  )
  # Prevent MSVC linker warning as error for __except + /guard:ehcont.
  target_compile_options(
    fmt
    PRIVATE
      $<$<BOOL:${WB_MSVC_ENABLE_GUARD_FOR_EH_CONTINUATION}>:/guard:ehcont>
  )
elseif (WB_COMPILER_CLANG OR WB_COMPILER_GCC)
  # Prevent linker warning "reloc which may overflow at runtime" (-fPIC).
  target_compile_options(fmt PRIVATE -fPIC)
endif()

# g3log.
add_subdirectory("deps/g3log")
if (WB_COMPILER_MSVC)
  # Prevent MSVC warning C4530: C++ exception handler used, but unwind
  # semantics are not enabled.
  target_compile_options(
    g3log
    PRIVATE
      /EH${WB_MSVC_EXCEPTION_HANDLING_MODEL}
  )
endif()
if (WB_OS_WIN)
  # Adds VERSION resource to g3log.
  wb_add_version_resource_to_dll_target(
    g3log
    deps/g3log
    "G3log is an asynchronous, crash-safe logger.  See THIRD_PARTY for details.")
endif()

# gtest / gmock.
if (WB_BUILD_TESTS)
  # Prevent overriding the parent project's compiler/linker settings.
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

  add_subdirectory("deps/googletest")

  # Do not define macroses which can clash with other libs.
  target_compile_definitions(gtest
    PRIVATE
      GTEST_DONT_DEFINE_FAIL=1
      GTEST_DONT_DEFINE_SUCCEED=1
      GTEST_DONT_DEFINE_TEST=1
  )

  if (WB_COMPILER_MSVC)
    # Prevent MSVC linker warning as error for __except + /guard:ehcont.
    target_compile_options(
      gtest
      PRIVATE
        $<$<BOOL:${WB_MSVC_ENABLE_GUARD_FOR_EH_CONTINUATION}>:/guard:ehcont>
    )
    target_link_options(
      gtest
      PRIVATE
        $<$<BOOL:${WB_MSVC_ENABLE_GUARD_FOR_EH_CONTINUATION}>:/guard:ehcont>
    )

    target_compile_options(
      gmock
      PRIVATE
        $<$<BOOL:${WB_MSVC_ENABLE_GUARD_FOR_EH_CONTINUATION}>:/guard:ehcont>
    )
    target_link_options(
      gmock
      PRIVATE
        $<$<BOOL:${WB_MSVC_ENABLE_GUARD_FOR_EH_CONTINUATION}>:/guard:ehcont>
    )

    target_compile_options(
      gmock_main
      PRIVATE
        $<$<BOOL:${WB_MSVC_ENABLE_GUARD_FOR_EH_CONTINUATION}>:/guard:ehcont>
    )
    target_link_options(
      gmock_main
      PRIVATE
        $<$<BOOL:${WB_MSVC_ENABLE_GUARD_FOR_EH_CONTINUATION}>:/guard:ehcont>
    )
  elseif (WB_COMPILER_CLANG OR WB_COMPILER_GCC)
    # Prevent linker warning "reloc which may overflow at runtime" (-fPIC).
    target_compile_options(gtest PRIVATE -fPIC)
    target_compile_options(gmock PRIVATE -fPIC)
  endif()
endif()

# abseil.

# Abseil libraries require C++11 as the current minimum standard.  Top-level
# application CMake projects should ensure a consistent C++ standard for all
# compiled sources by setting CMAKE_CXX_STANDARD.
set(ABSL_PROPAGATE_CXX_STD  ON)
set(WB_ABSL_BUILD_TESTS     OFF "Enable Abseil tests.")
# Abseil depends on gtest.
if (WB_ABSL_BUILD_TESTS)
  # Abseil will use gtest / gmock targets from source tree.
  set(ABSL_USE_EXTERNAL_GOOGLETEST  ON)
  set(ABSL_FIND_GOOGLETEST          OFF)
endif()
add_subdirectory("deps/abseil")
# Get all abseil targets to add overrides.
wb_get_all_targets(wb_all_abseil_targets "deps/abseil")
# Ensure all abseil targets use expected options & macroses.
foreach(wb_abseil_target_name ${wb_all_abseil_targets})
  if (${wb_abseil_target_name} MATCHES "absl_" AND
      NOT ${wb_abseil_target_name} STREQUAL "absl_civil_time" AND
      NOT ${wb_abseil_target_name} STREQUAL "absl_time_zone")
    get_target_property(wb_target_type ${wb_abseil_target_name} TYPE)

    if (NOT ("${wb_target_type}" STREQUAL "INTERFACE_LIBRARY"))
      if (WB_COMPILER_MSVC)
        # Warning C4530: C++ exception handler used, but unwind semantics are not
        # enabled.
        target_compile_options(
          ${wb_abseil_target_name}
          PRIVATE
            /EH${WB_MSVC_EXCEPTION_HANDLING_MODEL}
            $<$<BOOL:${WB_MSVC_ENABLE_GUARD_FOR_EH_CONTINUATION}>:/guard:ehcont>
        )
        # LNK2047: module contains C++ EH or complex EH metadata but was not
        # compiled with /guard:ehcont.
        target_link_options(
          ${wb_abseil_target_name}
          PRIVATE
            $<$<BOOL:${WB_MSVC_ENABLE_GUARD_FOR_EH_CONTINUATION}>:/guard:ehcont>
        )
      elseif(WB_COMPILER_CLANG OR WB_COMPILER_GCC)
        # Can't create dynamic relocation R_X86_64_32 against local symbol in
        # readonly segment.
        target_compile_options(${wb_abseil_target_name} PRIVATE -fPIC)
      endif()

      if (WB_BUILD_TESTS)
        # Abseil uses macroses without GTEST_ prefix.
        target_compile_definitions(${wb_abseil_target_name}
          INTERFACE
            GTEST_DONT_DEFINE_FAIL=0
            GTEST_DONT_DEFINE_SUCCEED=0
            GTEST_DONT_DEFINE_TEST=0
        )
      endif()
    endif()
  endif()
endforeach()

if (WB_COMPILER_MSVC)
  # absl_civil_time is special and requires PRIVATE options.
  target_compile_options(
    absl_civil_time
    PRIVATE
      /EH${WB_MSVC_EXCEPTION_HANDLING_MODEL}
      $<$<BOOL:${WB_MSVC_ENABLE_GUARD_FOR_EH_CONTINUATION}>:/guard:ehcont>
  )
  target_link_options(
    absl_civil_time
    PRIVATE
      $<$<BOOL:${WB_MSVC_ENABLE_GUARD_FOR_EH_CONTINUATION}>:/guard:ehcont>
  )

  # absl_time_zone is special and requires PRIVATE options.
  target_compile_options(
    absl_time_zone
    PRIVATE
      /EH${WB_MSVC_EXCEPTION_HANDLING_MODEL}
      $<$<BOOL:${WB_MSVC_ENABLE_GUARD_FOR_EH_CONTINUATION}>:/guard:ehcont>
  )
  target_link_options(
    absl_time_zone
    PRIVATE
      $<$<BOOL:${WB_MSVC_ENABLE_GUARD_FOR_EH_CONTINUATION}>:/guard:ehcont>
  )
elseif(WB_COMPILER_CLANG OR WB_COMPILER_GCC)
  # Can't create dynamic relocation R_X86_64_32 against local symbol in readonly
  # segment.
  target_compile_options(absl_civil_time PRIVATE -fPIC)
  target_compile_options(absl_time_zone  PRIVATE -fPIC)
endif()

# mimalloc.

add_subdirectory("deps/mimalloc")
if (WB_OS_WIN)
  # Adds VERSION resource.
  wb_add_version_resource_to_dll_target(
    mimalloc
    deps/mimalloc
    "Mimalloc is a general purpose allocator with excellent performance characteristics.  See THIRD_PARTY for details.")
endif()
if (MI_SECURE)
  # Inform we has mimalloc in secure mode.
  add_definitions(-DWB_MI_SECURE)
endif()

# SDL2 / sdl_image.

if (NOT WB_OS_WIN)
  set(SDL_STATIC OFF CACHE BOOL "Build static SDL library" FORCE)
  set(SDL_SHARED ON CACHE BOOL "Build shared SDL library" FORCE)

  add_subdirectory(deps/sdl)

  if (WB_CXX_COMPILER_ID STREQUAL "AppleClang")
    # Fix for undefined memset_pattern4() on MacOS.
    target_compile_definitions(SDL2 PUBLIC _DARWIN_C_SOURCE=1)
  endif()

  set(PNG_SHARED ON CACHE BOOL "Build shared lib" FORCE)
  set(PNG_STATIC OFF CACHE BOOL "Build static lib" FORCE)
  set(PNG_TESTS  OFF CACHE BOOL "Build libpng tests" FORCE)

  add_subdirectory(deps/sdl_image)

  # SDL image TIFF dependency forces other libraries to be build as shared.  Not that we want.
  set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build the library as a shared library" FORCE)

  if (WB_COMPILER_CLANG OR WB_COMPILER_GCC)
    # Linker error can't create dynamic relocation R_X86_64_64 against symbol:
    # XXX in readonly segment (-fPIC).
    target_compile_options(jpeg PRIVATE -fPIC)
    target_compile_options(SDL2_image PRIVATE -fPIC)
  endif()
endif()

## Libraries.

add_subdirectory("base")
add_subdirectory("whitebox-boot-manager")

## Hardware Abstraction Layer.

add_subdirectory("hal/drivers/hid")

## Kernel.

add_subdirectory("whitebox-kernel")
add_subdirectory("whitebox-ui")

## Apps.

add_subdirectory("apps/half-life-2")

## Other stuff.

# Get all targets to add mimalloc allocators overrides.
wb_get_all_targets(WB_ALL_TARGETS "${WB_ROOT_DIR}")

# Do some common stuff for all dependencies.
foreach(wb_target_name ${WB_ALL_TARGETS})
  get_target_property(wb_target_type ${wb_target_name} TYPE)

  # Only code gen targets.
  if (((${wb_target_type} MATCHES "LIBRARY" AND
        NOT ${wb_target_type} MATCHES "INTERFACE_LIBRARY") OR
       ${wb_target_type} MATCHES "EXECUTABLE"))
    # Ensure all libs use the same STL debug options.
    if (WB_COMPILER_CLANG OR WB_COMPILER_GCC)
      target_compile_definitions(${wb_target_name}
        PRIVATE
          $<$<CONFIG:DEBUG>:
            # Run-time bounds checking for C++ strings/containers.
            $<$<OR:$<BOOL:${WB_CLANG_DEFINE__GLIBCXX_ASSERTIONS}>,$<BOOL:${WB_CLANG_DEFINE__GLIBCXX_ASSERTIONS}>>:_GLIBCXX_ASSERTIONS>
            # Catch libstdc++ usage errors by enabling debug mode.  When defined,
            # _GLIBCXX_ASSERTIONS is defined automatically
            $<$<OR:$<BOOL:${WB_CLANG_DEFINE__GLIBCXX_ASSERTIONS}>,$<BOOL:${WB_GCC_DEFINE__GLIBCXX_ASSERTIONS}>>:_GLIBCXX_DEBUG>
            # When defined makes the debug mode extremely picky by making the use of
            # libstdc++ extensions and libstdc++-specific behavior into errors.
            $<$<OR:$<BOOL:${WB_CLANG_DEFINE__GLIBCXX_ASSERTIONS}>,$<BOOL:${WB_GCC_DEFINE__GLIBCXX_ASSERTIONS}>>:_GLIBCXX_DEBUG_PEDANTIC>
            # Enables special debugging checks meant to detect incorrect usage of the libc++ standard library.
            $<$<NOT:$<STREQUAL:"${WB_CLANG_DEFINE__LIBCPP_DEBUG}","">>:_LIBCPP_DEBUG=${WB_CLANG_DEFINE__LIBCPP_DEBUG}>
      >)
    endif()

    # Need to have pdbs on MSVC to simplify debugging.
    if (WB_OS_WIN)
      target_link_options(${wb_target_name}
        PRIVATE
          $<$<CONFIG:DEBUG>:
            # Generate a partial PDB file that simply references the original object
            # and library files.
            $<IF:$<BOOL:${WB_MSVC_ENABLE_DEBUG_FAST_LINK}>,/DEBUG:FASTLINK,/DEBUG:FULL>
          >

          ## Non-debug builds.
          $<$<NOT:$<CONFIG:DEBUG>>:
            # Generate full pdbs or not.
            $<IF:$<BOOL:${WB_MSVC_ENABLE_RELEASE_DEBUG_INFO}>,/DEBUG:FULL,/DEBUG:NONE>
          >
      )
    endif()

    # Do not add mimalloc / custom targets allocators to itself and some specific
    # actions.
    if (# ASAN intercepts new / delete.
        NOT ${wb_target_name} MATCHES "absl_leak_check_test" AND
        NOT ${wb_target_name} MATCHES "absl_disabled_leak_check_test" AND
        # Exclude itself.
        NOT ${wb_target_name} MATCHES "mimalloc")
      # Include the directories with the new files.
      target_include_directories(${wb_target_name}
        PRIVATE
          ${WB_ROOT_DIR}
          "${WB_ROOT_DIR}/deps/mimalloc/include")

      # Add new / delete allocator overrides.
      target_sources(${wb_target_name}
        PRIVATE
          ${WB_ROOT_DIR}/base/memory/memory_allocators_overrides.cc
      )

      # Add mimalloc to handle allocators.
      target_link_libraries(${wb_target_name} PRIVATE mimalloc)

      # mimalloc should be copied by each target individually as haven't found way
      # to it here.
    endif()
  endif()
endforeach()
