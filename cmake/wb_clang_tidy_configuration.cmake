# Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
# Use of this source code is governed by a 3-Clause BSD license that can be
# found in the LICENSE file.
#
# Clang-Tidy configuration.

option(WB_CLANG_TIDY_DIR "Define custom clang-tidy directory path." "")
option(WB_CLANG_TIDY_NAME "Determines clang-tidy executable name to use." "clang-tidy-13")
option(WB_GCC_ENABLE_CLANG_TIDY "Determines clang-tidy enabled in GCC builds.  Results in common set of Clang and GCC flags passed to GCC, which can lead to undetected source code warnings." OFF)

# Searches for clang-tidy and applies it for the target.
function(wb_apply_clang_tidy_options_to_target RETURN_VALUE THE_TARGET CXX_STANDARD)
  unset(${RETURN_VALUE})

  if (NOT WB_EXPORT_COMPILE_COMMANDS)
    message(AUTHOR_WARNING "${THE_TARGET} missed -DCMAKE_EXPORT_COMPILE_COMMANDS=ON option in cmake invocation.  Needed for running clang-tidy.")
  endif()

  if (WB_OS_WIN)
    get_filename_component(WB_CXX_COMPILER_PATH "${WB_CXX_COMPILER}" DIRECTORY CACHE)
    # Hack to allow find clang-tidy on Windows.
    set(WB_CLANG_TIDY_DIR "${WB_CXX_COMPILER_PATH}/../../../../../Llvm/x64/bin")

    message(STATUS "${THE_TARGET} clang-tidy directory: ${WB_CLANG_TIDY_DIR}.")
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
    message(STATUS "${THE_TARGET} clang-tidy not found.")

    set(${RETURN_VALUE} OFF PARENT_SCOPE)
  else()
    message(STATUS "${THE_TARGET} clang-tidy     found: ${WB_CLANG_TIDY}, setup one.")

    set(WB_CLANG_TIDY_OPTIONS
        "${WB_CLANG_TIDY}" "--checks=-*,cert-*;--extra-arg=-std=${CXX_STANDARD};--header-filter=^((?!deps).)*$;-p=${WB_BINARY_DIR};--warnings-as-errors=*")
    set_target_properties(${THE_TARGET} PROPERTIES CXX_CLANG_TIDY "${WB_CLANG_TIDY_OPTIONS}" )

    set(${RETURN_VALUE} ON PARENT_SCOPE)
  endif()
endfunction()