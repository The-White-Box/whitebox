# Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
# Use of this source code is governed by a 3-Clause BSD license that can be
# found in the LICENSE file.
#
# Clang-Tidy configuration.

option(WB_CLANG_TIDY_ENABLED "Enables clang-tidy checks." OFF)
option(WB_CLANG_TIDY_DIR "Define custom clang-tidy directory path." "")
option(WB_CLANG_TIDY_NAME "Determines clang-tidy executable name to use.")
# -bugprone-unhandled-exception-at-new -> if new throws we stop the app anyway.
# -modernize-use-trailing-return-type  -> questionable styling preferences.
set(WB_CLANG_TIDY_CHECKS
    "bugprone-*,-bugprone-unhandled-exception-at-new,cert-*,clang-analyzer-*,concurrency-*,misc-*,modernize-*,-modernize-use-trailing-return-type,performance-*,portability-*"
    CACHE STRING "Clang-tidy check groups to apply.  See https://clang.llvm.org/extra/clang-tidy/#using-clang-tidy")
option(WB_CLANG_TIDY_WARN_AS_ERROR "Enables clang-tidy warnings as errors mode" ON)
option(WB_GCC_ENABLE_CLANG_TIDY "Determines clang-tidy enabled in GCC builds.  Results in common set of Clang and GCC flags passed to GCC, which can lead to undetected source code warnings." OFF)

# Searches for clang-tidy and applies it for the target.
function(wb_apply_clang_tidy_options_to_target use_clang_tidy the_target cxx_standard)
  unset(${use_clang_tidy})

  if (NOT WB_CLANG_TIDY_ENABLED)
    message(STATUS "${the_target} clang-tidy is disabled.")
    set(${use_clang_tidy} OFF PARENT_SCOPE)
    return()
  endif()

  if (NOT WB_EXPORT_COMPILE_COMMANDS)
    message(AUTHOR_WARNING "${the_target} missed -DCMAKE_EXPORT_COMPILE_COMMANDS=ON option in cmake invocation.  Needed for running clang-tidy.")
  endif()

  if (WB_OS_WIN AND NOT WB_CLANG_TIDY_DIR)
    get_filename_component(cxx_compiler_path "${WB_CXX_COMPILER}" DIRECTORY CACHE)
    # Hack to allow find clang-tidy on Windows.
    set(WB_CLANG_TIDY_DIR "${cxx_compiler_path}/../../../../../Llvm/x64/bin")

    message(STATUS "${the_target} clang-tidy directory: ${WB_CLANG_TIDY_DIR}.")
  endif()

  if (NOT WB_CLANG_TIDY_DIR)
    find_program(WB_CLANG_TIDY
        NAMES "clang-tidy" "${WB_CLANG_TIDY_NAME}"
        DOC "Path to clang-tidy executable")
  else()
    find_program(WB_CLANG_TIDY
        NAMES "clang-tidy" "${WB_CLANG_TIDY_NAME}"
        PATHS "${WB_CLANG_TIDY_DIR}"
        DOC "Path to clang-tidy executable")
  endif()

  if (NOT WB_CLANG_TIDY)
    message(STATUS "${the_target} clang-tidy not found.")

    set(${use_clang_tidy} OFF PARENT_SCOPE)
  else()
    message(STATUS "${the_target} clang-tidy     found: ${WB_CLANG_TIDY}, setup one.")

    if (WB_CLANG_TIDY_WARN_AS_ERROR)
      set(warn_as_error ";--warnings-as-errors=*")
    else()
      set(warn_as_error "")
    endif()

    set(WB_CLANG_TIDY_OPTIONS
        "${WB_CLANG_TIDY}" "--checks=-*,${WB_CLANG_TIDY_CHECKS};--extra-arg=-std=${cxx_standard};--header-filter=^((?!deps).)*$;-p=${WB_BINARY_DIR}${warn_as_error}")
    set_target_properties(${the_target} PROPERTIES CXX_CLANG_TIDY "${WB_CLANG_TIDY_OPTIONS}" )

    set(${use_clang_tidy} ON PARENT_SCOPE)
  endif()
endfunction()