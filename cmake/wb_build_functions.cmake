# Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
# Use of this source code is governed by a 3-Clause BSD license that can be
# found in the LICENSE file.
#
# Functions to build exe / libraries / tests.

# Creates executable target.
#
# Usage example:
# wb_cxx_executable(
#   PROJECT_NAME  project_name
#   TARGET        target_name
#   VERSION       target_version
#   DESCRIPTION   target_description
#   SOURCE_DIR    source_dir
#   BINARY_DIR    binary_dir
#   CXX_DEFS      compiler_definitions
#   LINK_OPTS     linker_options
#   LINK_DEPS     linker_dependencies
#   RUNTIME_DEPS  runtime_dependencies
# )
function(wb_cxx_executable)
  cmake_parse_arguments(
    args
    ""
    "PROJECT_NAME;TARGET;VERSION;DESCRIPTION;SOURCE_DIR;BINARY_DIR"
    "CXX_DEFS;LINK_OPTS;LINK_DEPS;RUNTIME_DEPS"
    ${ARGN}
  )

  if (NOT args_PROJECT_NAME OR NOT args_TARGET OR NOT args_VERSION OR NOT args_DESCRIPTION)
    message(FATAL_ERROR
      "[build]: PROJECT_NAME & TARGET & VERSION & DESCRIPTION arguments are required.")
  endif()

  if (NOT args_SOURCE_DIR OR NOT args_BINARY_DIR)
    message(FATAL_ERROR
      "[build]: SOURCE_DIR & BINARY_DIR arguments are required.")
  endif()

  set(target_project_name  ${args_PROJECT_NAME})
  set(target_name          ${args_TARGET})
  set(target_version       ${args_VERSION})
  set(target_description   ${args_DESCRIPTION})
  set(target_source_dir    ${args_SOURCE_DIR})
  set(target_binary_dir    ${args_BINARY_DIR})

  project(
    ${target_project_name}
    VERSION ${target_version}
    DESCRIPTION ${target_description}
    LANGUAGES CXX
  )

  # Used in generated files.
  set(WB_CURRENT_TARGET_NAME ${target_name})

  # Used in generated files.
  if (WB_OS_WIN)
    set(WB_CURRENT_TARGET_SUFFIX ".exe")
  elseif (WB_OS_LINUX)
    set(WB_CURRENT_TARGET_SUFFIX "")
  elseif (NOT WB_OS_MACOSX)
    set(WB_CURRENT_TARGET_SUFFIX "")
  endif()

  # First find all test sources and header files.
  wb_auto_sources(header_files "*.h" "RECURSE" "${target_source_dir}")
  wb_auto_sources(source_files "*.cc" "RECURSE" "${target_source_dir}")

  # Exclude OS specific files.
  if (NOT WB_OS_WIN)
    wb_remove_matches_from_lists(header_files source_files
      MATCHES
        "^${target_source_dir}/win/"
        "^${target_source_dir}(.*)_win.cc"
    )
  endif()

  if (NOT WB_OS_LINUX)
    wb_remove_matches_from_lists(header_files source_files
      MATCHES
        # Technically Linux is not Unix, but more or less similar.
        "^${target_source_dir}/unix/"
        "^${target_source_dir}(.*)_unix.cc"
    )
  endif()

  if (NOT WB_OS_MACOSX)
    wb_remove_matches_from_lists(header_files source_files
      MATCHES
        "^${target_source_dir}/macos/"
        "^${target_source_dir}(.*)_macos.cc"
    )
  endif()

  # Remove tests.
  wb_remove_matches_from_lists(header_files source_files
    MATCHES
      "^${target_source_dir}(.*)?_tests.h$"
      "^${target_source_dir}(.*)?_tests.cc$"
  )

  # Generate project info.
  configure_file(
    ${WB_ROOT_DIR}/build/app_version_config.h.cmake.in
    ${target_binary_dir}/gen/app_version_config.h
  )

  if (WB_OS_WIN)
    add_executable(${target_name} WIN32 ${header_files} ${source_files})
  elseif (WB_OS_LINUX)
    add_executable(${target_name} ${header_files} ${source_files})
  elseif (WB_OS_MACOSX)
    add_executable(${target_name} MACOSX_BUNDLE ${header_files} ${source_files})
  endif()

  # Include the root and with generated info directories.
  target_include_directories(${target_name}
    PRIVATE
      ${WB_ROOT_DIR}
      ${target_binary_dir}/gen
      ${WB_BINARY_DIR}
  )

  # Append Windows specific.
  if (WB_OS_WIN)
    set(manifests_dir "${WB_ROOT_DIR}/build/win/manifests")
    # Generate visual styles manifest as requires project info.
    configure_file(
      ${manifests_dir}/enable-visual-styles.manifest.cmake
      ${WB_BINARY_DIR}/gen/enable-visual-styles.manifest
    )

    target_sources(${target_name}
      PRIVATE
        half_life_2_win.rc
        ${manifests_dir}/dpi-aware.manifest
        ${WB_BINARY_DIR}/gen/enable-visual-styles.manifest
        ${manifests_dir}/supported-os.manifest
        ${manifests_dir}/ultra-high-scroll-resolution.manifest
        ${manifests_dir}/utf-8-code-page.manifest
    )
  endif()

  # Specify project compile / link options.
  wb_apply_compile_options_to_target(${target_name})

  set(cxx_definitions  ${args_CXX_DEFS})
  target_compile_definitions(${target_name} PRIVATE ${cxx_definitions})

  set(link_options  ${args_LINK_OPTS})
  target_link_options(${target_name} PRIVATE ${link_options})

  set(link_dependencies  mimalloc ${args_LINK_DEPS})
  target_link_libraries(${target_name} PRIVATE ${link_dependencies})

  # Specify library version / soversion.
  set_target_properties(${target_name}
    PROPERTIES
      VERSION ${target_version}
      SOVERSION ${target_version}
  )

  set(runtime_dependencies mimalloc ${args_RUNTIME_DEPS})
  wb_copy_all_target_dependencies_to_target_bin_dir(${target_name}
    "${runtime_dependencies}")
endfunction(wb_cxx_executable)

# Creates shared library target.
#
# Usage example:
# wb_cxx_shared_library(
#   TARGET        target_name
#   VERSION       target_version
#   DESCRIPTION   target_description
#   SOURCE_DIR    source_dir
#   BINARY_DIR    binary_dir
#   CXX_DEFS      compiler_definitions
#   LINK_OPTS     linker_options
#   LINK_DEPS     linker_dependencies
#   RUNTIME_DEPS  runtime_dependencies
# )
function(wb_cxx_shared_library)
  cmake_parse_arguments(
    args
    ""
    "TARGET;VERSION;DESCRIPTION;SOURCE_DIR;BINARY_DIR"
    "CXX_DEFS;LINK_OPTS;LINK_DEPS;RUNTIME_DEPS"
    ${ARGN}
  )

  if (NOT args_TARGET OR NOT args_VERSION OR NOT args_DESCRIPTION)
    message(FATAL_ERROR
      "[build]: TARGET & VERSION & DESCRIPTION arguments are required.")
  endif()

  if (NOT args_SOURCE_DIR OR NOT args_BINARY_DIR)
    message(FATAL_ERROR
      "[build]: SOURCE_DIR & BINARY_DIR arguments are required.")
  endif()

  set(target_name         ${args_TARGET})
  set(target_version      ${args_VERSION})
  set(target_description  ${args_DESCRIPTION})
  set(target_source_dir   ${args_SOURCE_DIR})
  set(target_binary_dir   ${args_BINARY_DIR})

  project(
    ${target_name}
    VERSION ${target_version}
    DESCRIPTION ${target_description}
    LANGUAGES CXX
  )

  # Used in generated files.
  set(WB_CURRENT_TARGET_NAME ${target_name})

  # Used in generated files.
  if (WB_OS_WIN)
    set(WB_CURRENT_TARGET_SUFFIX ".dll")
  elseif (WB_OS_LINUX)
    set(WB_CURRENT_TARGET_SUFFIX ".so")
  elseif (NOT WB_OS_MACOSX)
    set(WB_CURRENT_TARGET_SUFFIX ".dylib")
  endif()

  # First find all test sources and header files.
  wb_auto_sources(header_files "*.h" "RECURSE" "${target_source_dir}")
  wb_auto_sources(source_files "*.cc" "RECURSE" "${target_source_dir}")

  # Exclude OS specific files.
  if (NOT WB_OS_MACOSX)
    wb_remove_matches_from_lists(header_files source_files
      MATCHES
        "^${target_source_dir}/macos/"
        "^${target_source_dir}(.*)_macos.cc"
    )
  endif()

  if (NOT WB_OS_LINUX)
    wb_remove_matches_from_lists(header_files source_files
      MATCHES
        # Technically Linux is not Unix, but more or less similar.
        "^${target_source_dir}/unix/"
        "^${target_source_dir}(.*)_unix.cc"
    )
  endif()

  if (NOT WB_OS_WIN)
    wb_remove_matches_from_lists(header_files source_files
      MATCHES
        "^${target_source_dir}/win/"
        "^${target_source_dir}(.*)_win.cc"
    )
  endif()

  # Remove tests.
  wb_remove_matches_from_lists(header_files source_files
    MATCHES
      "^${target_source_dir}(.*)?_tests(_macos|_unix|_win)?.h$"
      "^${target_source_dir}(.*)?_tests(_macos|_unix|_win)?.cc$"
  )

  # Generate project info.
  configure_file(
    ${WB_ROOT_DIR}/build/app_version_config.h.cmake.in
    ${target_binary_dir}/gen/app_version_config.h
  )

  add_library(${target_name} SHARED ${header_files} ${source_files})

  # Include the root and with generated info directories.
  target_include_directories(${target_name}
    PRIVATE
      ${WB_ROOT_DIR}
      ${target_binary_dir}/gen
      ${WB_BINARY_DIR}
  )

  # Specify project compile / link options.
  wb_apply_compile_options_to_target(${target_name})

  set(cxx_definitions  ${args_CXX_DEFS})
  target_compile_definitions(${target_name} PRIVATE ${cxx_definitions})

  set(link_options  ${args_LINK_OPTS})
  target_link_options(${target_name} PRIVATE ${link_options})

  set(link_dependencies  mimalloc ${args_LINK_DEPS})
  target_link_libraries(${target_name} PRIVATE ${link_dependencies})

  # Specify library version / soversion.
  set_target_properties(${target_name}
    PROPERTIES
      VERSION ${target_version}
      SOVERSION ${target_version}
  )

  # Append Windows specific.
  if (WB_OS_WIN)
    target_sources(${target_name}
      PRIVATE
        ${WB_ROOT_DIR}/build/win/resource_scripts/windows_dll_base.rc
    )
  endif()

  set(runtime_dependencies mimalloc ${args_RUNTIME_DEPS})
  wb_copy_all_target_dependencies_to_target_bin_dir(${target_name}
    "${runtime_dependencies}")
endfunction(wb_cxx_shared_library)

# Creates GTest test executable with tests from target.
#
# Usage example:
# wb_cxx_test_exe_for_target(
#   TARGET        testing_target
#   SOURCE_DIR    tests_source_directory
#   LINK_DEPS?    tests_link_dependencies
#   RUNTIME_DEPS? tests_runtime_dependencies
# )
function(wb_cxx_test_exe_for_target)
  cmake_parse_arguments(
    args
    ""
    "TARGET;SOURCE_DIR"
    "LINK_DEPS;RUNTIME_DEPS"
    ${ARGN}
  )

  if (NOT args_TARGET OR NOT args_SOURCE_DIR)
    message(FATAL_ERROR "[tests]: TARGET & SOURCE_DIR arguments are required.")
  endif()
  
  set(target_name         ${args_TARGET})
  set(target_source_dir   ${args_SOURCE_DIR})

  # First find all test sources and header files.
  wb_auto_sources(header_files "*_tests*.h" "RECURSE" "${target_source_dir}")
  wb_auto_sources(source_files "*_tests*.cc" "RECURSE" "${target_source_dir}")

  # Exclude OS specific files.
  if (NOT WB_OS_MACOSX)
    wb_remove_matches_from_lists(header_files source_files
      MATCHES
        "^${target_source_dir}/macos/"
        "^${target_source_dir}(.*)_macos.cc"
    )
  endif()

  if (NOT WB_OS_LINUX)
    wb_remove_matches_from_lists(header_files source_files
      MATCHES
        # Technically Linux is not Unix, but more or less similar.
        "^${target_source_dir}/unix/"
        "^${target_source_dir}(.*)_unix.cc"
    )
  endif()

  if (NOT WB_OS_WIN)
    wb_remove_matches_from_lists(header_files source_files
        MATCHES
        "^${target_source_dir}/win/"
        "^${target_source_dir}(.*)_win.cc"
        )
  endif ()

  set(tests_target_name "${target_name}_tests")
  add_executable(${tests_target_name} ${header_files} ${source_files})

  # Include the root and with generated info directories.
  target_include_directories(${tests_target_name}
      PRIVATE
      ${WB_ROOT_DIR}
      ${target_binary_dir}/gen
      )

  # Specify tests compile / link options.
  wb_apply_compile_options_to_target(${tests_target_name})

  set(target_link_dependencies ${args_LINK_DEPS})
  set(target_runtime_dependencies ${args_RUNTIME_DEPS})

  set(tests_link_dependencies GTest::gtest_main mimalloc)
  list(APPEND tests_link_dependencies ${target_link_dependencies} ${target_name})
  target_link_libraries(${tests_target_name} PRIVATE ${tests_link_dependencies})

  set(tests_runtime_dependencies mimalloc)
  list(APPEND tests_runtime_dependencies ${target_runtime_dependencies})
  wb_copy_all_target_dependencies_to_target_bin_dir(${tests_target_name}
    "${tests_runtime_dependencies}")

  include(GoogleTest)
  gtest_discover_tests(${tests_target_name})
endfunction(wb_cxx_test_exe_for_target)
