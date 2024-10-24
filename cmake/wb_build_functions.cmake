# Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
# Use of this source code is governed by a 3-Clause BSD license that can be
# found in the LICENSE file.
#
# Functions to build exe / libraries / tests.

# Creates executable target.
#
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
  set(WB_CURRENT_TARGET_DESCRIPTION ${target_description})

  wb_exe_suffix(WB_CURRENT_TARGET_SUFFIX)

  # First find all test sources and header files.
  wb_auto_sources(header_files "*.h" "RECURSE" "${target_source_dir}")
  wb_auto_sources(source_files "*.cc" "RECURSE" "${target_source_dir}")
  wb_remove_os_specific_files(${target_source_dir}
    "${header_files}" "${source_files}" ON)

  # Generate project info.
  configure_file(
    ${WB_ROOT_DIR}/build/app_version_config.h.cmake.in
    ${target_binary_dir}/gen/app_version_config.h
  )

  if (WB_OS_WIN)
    add_executable(${target_name} WIN32 ${source_files} ${header_files})
  elseif (WB_OS_LINUX)
    add_executable(${target_name} ${source_files} ${header_files})
  elseif (WB_OS_MACOS)
    add_executable(${target_name} MACOSX_BUNDLE ${source_files} ${header_files})
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
      ${target_binary_dir}/gen/enable-visual-styles.manifest
    )

    target_sources(${target_name}
      PRIVATE
        half_life_2_win.rc
        ${manifests_dir}/dpi-aware.manifest
        ${target_binary_dir}/gen/enable-visual-styles.manifest
        ${manifests_dir}/supported-os.manifest
        ${manifests_dir}/ultra-high-scroll-resolution.manifest
        ${manifests_dir}/utf-8-code-page.manifest
    )
  endif()

  # To see what is actually included.
  message(STATUS "${target_name} has header files:")
  foreach (header_file ${header_files})
    message(STATUS "${header_file}")
  endforeach()

  message(STATUS "${target_name} has source files:")
  foreach (source_file ${source_files})
    message(STATUS "${source_file}")
  endforeach()

  # Specify project compile / link options.
  wb_apply_compile_options_to_target(${target_name})

  set(cxx_definitions  ${args_CXX_DEFS})
  target_compile_definitions(${target_name} PRIVATE ${cxx_definitions})

  set(link_options  ${args_LINK_OPTS})
  target_link_options(${target_name} PRIVATE ${link_options})

  set(link_dependencies  ${args_LINK_DEPS})
  target_link_libraries(${target_name} PRIVATE ${link_dependencies})

  # Specify library version / soversion.
  set_target_properties(${target_name}
    PROPERTIES
      VERSION ${target_version}
      SOVERSION ${target_version}
  )

  set(runtime_dependencies  ${args_RUNTIME_DEPS})
  wb_copy_all_target_dependencies_to_target_bin_dir(${target_name}
    "${runtime_dependencies}")
endfunction(wb_cxx_executable)

# Creates shared library target.
#
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
  set(WB_CURRENT_TARGET_DESCRIPTION ${target_description})

  # Used in generated files.
  wb_shared_library_suffix(WB_CURRENT_TARGET_SUFFIX)

  # First find all test sources and header files.
  wb_auto_sources(header_files "*.h" "RECURSE" "${target_source_dir}")
  wb_auto_sources(source_files "*.cc" "RECURSE" "${target_source_dir}")
  wb_remove_os_specific_files(${target_source_dir}
    "${header_files}" "${source_files}" ON)

  # To see what is actually included.
  message(STATUS "${target_name} has header files:")
  foreach (header_file ${header_files})
    message(STATUS "${header_file}")
  endforeach()

  message(STATUS "${target_name} has source files:")
  foreach (source_file ${source_files})
    message(STATUS "${source_file}")
  endforeach()

  # Generate project info.
  configure_file(
    ${WB_ROOT_DIR}/build/app_version_config.h.cmake.in
    ${target_binary_dir}/gen/app_version_config.h
  )

  add_library(${target_name} SHARED ${source_files} ${header_files})
  # Alias target for in-tree builds.
  add_library("wb::${target_name}" ALIAS ${target_name})

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

  set(link_dependencies  ${args_LINK_DEPS})
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

  set(runtime_dependencies  ${args_RUNTIME_DEPS})
endfunction(wb_cxx_shared_library)

# Creates GTest test executable with tests from target.
#
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
  wb_remove_os_specific_files(${target_source_dir}
    "${header_files}" "${source_files}" OFF)

  set(tests_target_name "${target_name}_tests")
  add_executable(${tests_target_name} ${source_files} ${header_files})

  # To see what is actually included.
  message(STATUS "${tests_target_name} has header files:")
  foreach (header_file ${header_files})
    message(STATUS "${header_file}")
  endforeach()

  message(STATUS "${tests_target_name} has source files:")
  foreach (source_file ${source_files})
    message(STATUS "${source_file}")
  endforeach()

  # Include the root directory.
  target_include_directories(${tests_target_name}
    PRIVATE
      ${WB_ROOT_DIR}
  )

  # Specify tests compile / link options.
  wb_apply_compile_options_to_target(${tests_target_name})

  set(target_link_dependencies ${args_LINK_DEPS})
  set(target_runtime_dependencies ${args_RUNTIME_DEPS})

  set(tests_link_dependencies GTest::gtest)
  list(APPEND tests_link_dependencies ${target_link_dependencies} ${target_name})
  target_link_libraries(${tests_target_name} PRIVATE ${tests_link_dependencies})

  set(tests_runtime_dependencies "")
  list(APPEND tests_runtime_dependencies ${target_runtime_dependencies})
  wb_copy_all_target_dependencies_to_target_bin_dir(${tests_target_name}
    "${tests_link_dependencies}")

  include(GoogleTest)
  gtest_discover_tests(${tests_target_name})
endfunction(wb_cxx_test_exe_for_target)
