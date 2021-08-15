# Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
# Use of this source code is governed by a 3-Clause BSD license that can be
# found in the LICENSE file.
#
# Functions to build exe / libraries / tests.

# Creates GTest test executable with tests from target.
#
# Usage example:
# wb_cxx_test_exe_for_target(
#   TARGET        testing_target
#   SRC_DIR       tests_source_directory
#   LINK_DEPS?    tests_link_dependencies
#   RUNTIME_DEPS? tests_runtime_dependencies
# )
function(wb_cxx_test_exe_for_target)
  cmake_parse_arguments(
    args
    ""
    "TARGET;SRC_DIR"
    "LINK_DEPS;RUNTIME_DEPS"
    ${ARGN}
  )

  if (NOT args_TARGET OR NOT args_SRC_DIR)
    message(FATAL_ERROR
      "[tests]: wb_cxx_test_exe's TARGET & SRC_DIR arguments are required.")
  endif()

  # First find all test sources and header files.
  wb_auto_sources(tests_files "*_tests.cc" "RECURSE" "${args_SRC_DIR}")

  # Exclude OS specific files.
  if (NOT WB_OS_WIN)
    wb_remove_matches_from_lists(tests_files
      MATCHES
        "^${args_SRC_DIR}/windows/"
    )
  elseif (NOT WB_OS_LINUX)
    wb_remove_matches_from_lists(tests_files
      MATCHES
        "^${args_SRC_DIR}/linux/"
    )
  elseif (NOT WB_OS_MACOSX)
    wb_remove_matches_from_lists(tests_files
      MATCHES
        "^${args_SRC_DIR}/macos/"
    )
  endif()

  set(tests_target_name "${args_TARGET}_tests")
  add_executable(${tests_target_name} ${tests_files})

  # Specify tests compile / link options.
  wb_apply_compile_options_to_target(${tests_target_name})

  set(tests_link_dependencies GTest::gtest_main mimalloc)
  list(APPEND tests_link_dependencies ${args_LINK_DEPS} ${args_TARGET})
  target_link_libraries(${tests_target_name} PRIVATE ${tests_link_dependencies})

  set(tests_runtime_dependencies mimalloc)
  list(APPEND tests_runtime_dependencies ${args_RUNTIME_DEPS})
  wb_copy_all_target_dependencies_to_target_bin_dir(${tests_target_name}
    "${tests_runtime_dependencies}")

  include(GoogleTest)
  gtest_discover_tests(${tests_target_name})
endfunction(wb_cxx_test_exe_for_target)