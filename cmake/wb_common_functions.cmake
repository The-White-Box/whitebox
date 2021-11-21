# Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
# Use of this source code is governed by a 3-Clause BSD license that can be
# found in the LICENSE file.
#
# Useful generic functions.
#
# Derived from https://github.com/facebook/folly/blob/master/CMake/FollyFunctions.cmake
# which is licensed under the Apache License, Version 2.0 (the "License").
# See the License for the specific language governing permissions and
# limitations under the License.

# Fill first arg with list of values by pattern.
#
# wb_auto_sources(files "*.cc" "RECURSE" "${SRC_DIR}")
function(wb_auto_sources RETURN_VALUE PATTERN SOURCE_SUBDIRS)
  if ("${SOURCE_SUBDIRS}" STREQUAL "RECURSE")
    SET(PATH ".")
    if (${ARGC} EQUAL 4)
      list(GET ARGV 3 PATH)
    endif()
  endif()

  if ("${SOURCE_SUBDIRS}" STREQUAL "RECURSE")
    unset(${RETURN_VALUE})
    file(GLOB SUBDIR_FILES "${PATH}/${PATTERN}")
    list(APPEND ${RETURN_VALUE} ${SUBDIR_FILES})

    file(GLOB subdirs RELATIVE ${PATH} ${PATH}/*)

    foreach(DIR ${subdirs})
      if (IS_DIRECTORY ${PATH}/${DIR})
        if (NOT "${DIR}" STREQUAL "CMakeFiles")
          file(GLOB_RECURSE SUBDIR_FILES "${PATH}/${DIR}/${PATTERN}")
          list(APPEND ${RETURN_VALUE} ${SUBDIR_FILES})
        endif()
      endif()
    endforeach()
  else()
    file(GLOB ${RETURN_VALUE} "${PATTERN}")

    foreach (PATH ${SOURCE_SUBDIRS})
      file(GLOB SUBDIR_FILES "${PATH}/${PATTERN}")
      list(APPEND ${RETURN_VALUE} ${SUBDIR_FILES})
    endforeach()
  endif()

  set(${RETURN_VALUE} ${${RETURN_VALUE}} PARENT_SCOPE)
endfunction(wb_auto_sources)

# Remove all files matching a set of patterns, and,
# optionally, not matching a second set of patterns,
# from a set of lists.
#
# Example:
# This will remove all files in the CPP_SOURCES list
# matching "/test/" or "Test.cpp$", but not matching
# "BobTest.cpp$".
# wb_remove_matches_from_lists(CPP_SOURCES MATCHES "/test/" "Test.cpp$" IGNORE_MATCHES "BobTest.cpp$")
#
# Parameters:
#
# [...]:
# The names of the lists to remove matches from.
#
# [MATCHES ...]:
# The matches to remove from the lists.
#
# [IGNORE_MATCHES ...]:
# The matches not to remove, even if they match
# the main set of matches to remove.
function(wb_remove_matches_from_lists)
  set(LISTS_TO_SEARCH)
  set(MATCHES_TO_REMOVE)
  set(MATCHES_TO_IGNORE)
  set(argumentState 0)
  foreach (arg ${ARGN})
    if ("x${arg}" STREQUAL "xMATCHES")
      set(argumentState 1)
    elseif ("x${arg}" STREQUAL "xIGNORE_MATCHES")
      set(argumentState 2)
    elseif (argumentState EQUAL 0)
      list(APPEND LISTS_TO_SEARCH ${arg})
    elseif (argumentState EQUAL 1)
      list(APPEND MATCHES_TO_REMOVE ${arg})
    elseif (argumentState EQUAL 2)
      list(APPEND MATCHES_TO_IGNORE ${arg})
    else()
      message(FATAL_ERROR "Unknown argument state!")
    endif()
  endforeach()

  foreach (theList ${LISTS_TO_SEARCH})
    foreach (entry ${${theList}})
      foreach (match ${MATCHES_TO_REMOVE})
        if (${entry} MATCHES ${match})
          set(SHOULD_IGNORE OFF)
          foreach (ign ${MATCHES_TO_IGNORE})
            if (${entry} MATCHES ${ign})
              set(SHOULD_IGNORE ON)
              break()
            endif()
          endforeach()

          if (NOT SHOULD_IGNORE)
            list(REMOVE_ITEM ${theList} ${entry})
          endif()
        endif()
      endforeach()
    endforeach()
    set(${theList} ${${theList}} PARENT_SCOPE)
  endforeach()
endfunction()

# Removes OS specific files from target.
#
# wb_remove_os_specific_files(
#   "target_source_dir"
#   "${header_files}"
#   "${source_files}"
#   ON | OFF
# )
function(wb_remove_os_specific_files target_source_dir header_files source_files
    should_exclude_tests)
  # Exclude OS specific files.
  if (NOT WB_OS_WIN)
    wb_remove_matches_from_lists(header_files source_files
      MATCHES
        "^${target_source_dir}/win/"
        "^${target_source_dir}(.*)_win.h"
        "^${target_source_dir}(.*)_win.cc"
    )
  endif()

  if (NOT WB_OS_LINUX)
    wb_remove_matches_from_lists(header_files source_files
      MATCHES
        # Technically Linux is not Unix, but more or less similar.
        "^${target_source_dir}/unix/"
        "^${target_source_dir}(.*)_unix.h"
        "^${target_source_dir}(.*)_unix.cc"
    )
  endif()

  if (NOT WB_OS_MACOS)
    wb_remove_matches_from_lists(header_files source_files
      MATCHES
        "^${target_source_dir}/macos/"
        "^${target_source_dir}(.*)_macos.h"
        "^${target_source_dir}(.*)_macos.cc"
    )
  endif()

  # Remove tests.
  if (should_exclude_tests)
    wb_remove_matches_from_lists(header_files source_files
      MATCHES
        "^${target_source_dir}(.*)?_tests(_macos|_unix|_win)?.h$"
        "^${target_source_dir}(.*)?_tests(_macos|_unix|_win)?.cc$"
    )
  endif()

  set(header_files ${header_files} PARENT_SCOPE)
  set(source_files ${source_files} PARENT_SCOPE)
endfunction()

# Prints bool option value.
function(wb_bool_option name value)
  if(${value})
    message(STATUS "  ${name}:\tON")
  else()
    message(STATUS "  ${name}:\tOFF")
  endif()
endfunction()

# Checks platform requirements.
#
# wb_check_platform_requirements("My target name")
function(wb_check_platform_requirements TARGET_NAME)
  # Check target architecture is 64bit, as we support only 64bit+.
  if (NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR
      "[common]: ${TARGET_NAME} requires 64 bit host os but got ${CMAKE_HOST_SYSTEM}/${CMAKE_HOST_SYSTEM_PROCESSOR}.")
  endif()

  message(STATUS "[common]: ${TARGET_NAME} building on ${CMAKE_HOST_SYSTEM}/${CMAKE_HOST_SYSTEM_PROCESSOR}/64 bit.")

  # At least Visual Studio 2019 version 16.8 as we use latest C++17 features.
  if (CMAKE_SYSTEM_NAME STREQUAL "Windows" AND MSVC_VERSION LESS 1928)
    message(
      FATAL_ERROR
      "[common]: This build script only supports building '${TARGET_NAME}' on 64-bit "
      "Windows with at least Visual Studio 2019 version 16.8. "
      "MSVC version '${MSVC_VERSION}' is not supported."
    )
  endif()
endfunction(wb_check_platform_requirements)

# Checks cxx source compiles.  Defines |define_name| to 1 when source compiles.
function(wb_check_cxx_compiles source_code define_name source_description)
  check_cxx_source_compiles(source_code define_name)

  if (define_name)
    add_compile_definitions(${define_name}=1)
  endif()

  wb_bool_option(
    "[cpp features]: cxx compiler ${WB_CXX_COMPILER_ID} ${source_description} support" define_name)
endfunction()

# Alas, option() doesn't support string values.
#
# wb_define_strings_option(MSVC_LANGUAGE_VERSION
#   "This determines which version of C++ to compile as."
#   "c++17" "c++latest")
function(wb_define_strings_option OPTIONS_VAR_NAME OPTIONS_VAR_DESCRIPTION DEFAULT_OPTION)
  set(ALLOWED_OPTIONS_SET "'${DEFAULT_OPTION}'")

  foreach(ALLOWED_OPTION IN LISTS ARGN)
    string(APPEND ALLOWED_OPTIONS_SET ", '${ALLOWED_OPTION}'")
  endforeach()

  if (NOT DEFINED ${OPTIONS_VAR_NAME})
    set(${OPTIONS_VAR_NAME} "${DEFAULT_OPTION}" PARENT_SCOPE)
  else()
    set(${OPTIONS_VAR_NAME} ${${OPTIONS_VAR_NAME}} PARENT_SCOPE)
  endif()

  message(STATUS "[options]: ${OPTIONS_VAR_NAME} is ${${OPTIONS_VAR_NAME}}")

  set(${OPTIONS_VAR_NAME} "${DEFAULT_OPTION}" CACHE STRING "One of ${ALLOWED_OPTIONS_SET}. ${OPTIONS_VAR_DESCRIPTION}")
  # Add a pretty drop-down selector for these values when using the GUI.
  set_property(
    CACHE ${OPTIONS_VAR_NAME}
    PROPERTY STRINGS
      ${DEFAULT_OPTION}
      ${ARGN}
      PARENT_SCOPE
  )

  set(OPTION_VALUE ${${OPTIONS_VAR_NAME}})
  set(OPTION_VALID OFF)

  # Validate.
  if ("${OPTION_VALUE}" STREQUAL "${DEFAULT_OPTION}")
    set(OPTION_VALID ON)
  else()
    foreach(ALLOWED_OPTION IN LISTS ARGN)
      if ("${OPTION_VALUE}" STREQUAL "${ALLOWED_OPTION}")
        set(OPTION_VALID ON)
        break()
      endif()
    endforeach()
  endif()

  # Then add.
  if (NOT OPTION_VALID)
    message(FATAL_ERROR
      "${OPTIONS_VAR_NAME} must be set to one of ${ALLOWED_OPTIONS_SET}! "
      "Got '${${OPTIONS_VAR_NAME}}' instead!")
  endif()
endfunction(wb_define_strings_option)

# Dumps target property as message.
#
# wb_dump_target_property("My target" "My property" "My property description")
function(wb_dump_target_property THE_TARGET TARGET_PROPERTY TARGET_PROPERTY_DESCRIPTION)
  get_target_property(TARGET_PROPERTY_VALUE ${THE_TARGET} ${TARGET_PROPERTY})

  message("-- ${THE_TARGET} ${TARGET_PROPERTY_DESCRIPTION}: ${TARGET_PROPERTY_VALUE}")

  # TODO(dimhotepus): Encode echo argument.
  # add_custom_command(TARGET ${THE_TARGET}
  #  POST_BUILD
  #  COMMAND echo "${THE_TARGET} built with the ${TARGET_PROPERTY_DESCRIPTION}: ${TARGET_PROPERTY_VALUE}")
endfunction(wb_dump_target_property)

# Copies target assets to target binary directory.
# 
# wb_copy_target_asset_to_target_bin_dir("My target" "My asset")
function(wb_copy_target_asset_to_target_bin_dir THE_TARGET THE_ASSET_RELATIVE_PATH)
  set(full_asset_path "${WB_ROOT_DIR}/assets/${THE_TARGET}/${THE_ASSET_RELATIVE_PATH}")
  add_custom_command(
      TARGET ${THE_TARGET} POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E copy_if_different ${full_asset_path} $<TARGET_FILE_DIR:${THE_TARGET}>
      WORKING_DIRECTORY $<TARGET_FILE_DIR:${THE_TARGET}>
      COMMENT "Copy ${full_asset_path} to $<TARGET_FILE_DIR:${THE_TARGET}> output directory"
  )
endfunction()

# Copies target licenses to target binary directory.
#
# wb_copy_licenses_to_target_bin_dir("My target")
function(wb_copy_licenses_to_target_bin_dir THE_TARGET)
  set(license_path "${WB_ROOT_DIR}/LICENSE")
  set(third_party_license_path "${WB_ROOT_DIR}/THIRD_PARTY")
  add_custom_command(
      TARGET ${THE_TARGET} POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E copy_if_different ${license_path} $<TARGET_FILE_DIR:${THE_TARGET}>
      WORKING_DIRECTORY $<TARGET_FILE_DIR:${THE_TARGET}>
      COMMENT "Copy ${license_path} to $<TARGET_FILE_DIR:${THE_TARGET}> output directory"
  )
  add_custom_command(
      TARGET ${THE_TARGET} POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E copy_if_different ${third_party_license_path} $<TARGET_FILE_DIR:${THE_TARGET}>
      WORKING_DIRECTORY $<TARGET_FILE_DIR:${THE_TARGET}>
      COMMENT "Copy ${third_party_license_path} to $<TARGET_FILE_DIR:${THE_TARGET}> output directory"
  )
endfunction()

# Copies target dependency to target binary dir.
#
# wb_copy_target_dependency_to_target_bin_dir("My target" "My dependency")
function(wb_copy_target_dependency_to_target_bin_dir THE_TARGET THE_DEPENDENCY)
  add_custom_command(
    TARGET ${THE_TARGET} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different $<TARGET_FILE:${THE_DEPENDENCY}> $<TARGET_FILE_DIR:${THE_TARGET}>
    DEPENDS ${THE_DEPENDENCY}
    WORKING_DIRECTORY $<TARGET_FILE_DIR:${THE_TARGET}>
    COMMENT "Copy $<TARGET_FILE:${THE_DEPENDENCY}> to $<TARGET_FILE_DIR:${THE_TARGET}> output directory"
  )
endfunction()

# Copy all target dependencies + optionally mimalloc redirect.
#
# wb_copy_all_target_dependencies_to_target_bin_dir("My target" My_dependencies_list)
function(wb_copy_all_target_dependencies_to_target_bin_dir THE_TARGET THE_DEPENDENCIES)
  set(WB_SHOULD_COPY_MIMALLOC OFF)

  # Add runtime dependencies.
  foreach(THE_DEPENDENCY ${THE_DEPENDENCIES})
    wb_copy_target_dependency_to_target_bin_dir(${THE_TARGET} ${THE_DEPENDENCY})

    if ("${THE_DEPENDENCY}" STREQUAL "mimalloc")
      set(WB_SHOULD_COPY_MIMALLOC ON)
    endif()
  endforeach()

  if (WB_SHOULD_COPY_MIMALLOC AND WB_OS_WIN AND MI_BUILD_SHARED)
    # On windows copy the mimalloc redirection dll too.
    add_custom_command(
      TARGET ${THE_TARGET} POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE_DIR:mimalloc>/mimalloc-redirect.dll" $<TARGET_FILE_DIR:${THE_TARGET}>
      DEPENDS ${THE_DEPENDENCY}
      COMMENT "Copy $<TARGET_FILE_DIR:mimalloc>/mimalloc-redirect.dll to $<TARGET_FILE_DIR:${THE_TARGET} output directory"
    )
  endif()
endfunction()

# Adds VERSION resource to Win32 DLL target.
#
# wb_add_version_resource_to_dll_target("My target" "Target bin dir path" "Target description")
function(wb_add_version_resource_to_dll_target THE_TARGET THE_TARGET_DIR_PATH THE_TARGET_DESCRIPTION)
  set(target_bin_dir ${WB_BINARY_DIR}/${THE_TARGET_DIR_PATH})

  # Used in generated files.
  set(WB_CURRENT_TARGET_NAME ${THE_TARGET})
  set(WB_CURRENT_TARGET_DESCRIPTION ${THE_TARGET_DESCRIPTION})

  if (WB_OS_WIN)
    set(WB_CURRENT_TARGET_SUFFIX ".dll")
  elseif (WB_OS_LINUX)
    set(WB_CURRENT_TARGET_SUFFIX ".so")
  elseif (NOT WB_OS_MACOS)
    set(WB_CURRENT_TARGET_SUFFIX ".dylib")
  endif()

  # Generate VERSION config headers.
  configure_file(
    ${WB_ROOT_DIR}/build/app_version_config.h.cmake.in
    ${target_bin_dir}/gen/app_version_config.h
  )
  # Include required VERSION config directories.
  target_include_directories(${THE_TARGET}
    PRIVATE
      ${WB_ROOT_DIR}
      ${target_bin_dir}/gen
      ${WB_BINARY_DIR}
  )
  # Add resource compiler script to target.
  target_sources(${THE_TARGET}
    PRIVATE
      ${WB_ROOT_DIR}/build/win/resource_scripts/windows_dll_base.rc
  )
endfunction()

# Collect all currently added targets in all subdirectories
#
# Parameters:
# - RESULT the list containing all found targets
# - ROOT_DIR root directory to start looking from
#
# See https://stackoverflow.com/questions/60211516/programmatically-get-all-targets-in-a-cmake-project
function(wb_get_all_targets RESULT ROOT_DIR)
  get_property(SUB_DIRS DIRECTORY "${ROOT_DIR}" PROPERTY SUBDIRECTORIES)

  foreach(SUB_DIR IN LISTS SUB_DIRS)
    wb_get_all_targets(${RESULT} "${SUB_DIR}")
  endforeach()

  get_directory_property(SUB_TARGETS DIRECTORY "${ROOT_DIR}" BUILDSYSTEM_TARGETS)
  set(${RESULT} ${${RESULT}} ${SUB_TARGETS} PARENT_SCOPE)
endfunction()
