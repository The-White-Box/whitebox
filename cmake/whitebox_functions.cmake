# Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
# Use of this source code is governed by a 3-Clause BSD license that can be
# found in the LICENSE file.
#
# Derived from https://github.com/facebook/folly/blob/master/CMake/FollyFunctions.cmake
# which is licensed under the Apache License, Version 2.0 (the "License").
# See the License for the specific language governing permissions and
# limitations under the License.

# Use like this: auto_sources(files "*.cc" "RECURSE" "${SRC_DIR}")
function(auto_sources RETURN_VALUE PATTERN SOURCE_SUBDIRS)
  if ("${SOURCE_SUBDIRS}" STREQUAL "RECURSE")
    SET(PATH ".")
    if (${ARGC} EQUAL 4)
      list(GET ARGV 3 PATH)
    endif ()
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
  endif ()

  set(${RETURN_VALUE} ${${RETURN_VALUE}} PARENT_SCOPE)
endfunction(auto_sources)

# Use like this: verify_target_architecture("My target name")
function(verify_target_architecture TARGET_NAME)
  # Check target architecture is 64bit, as we support only 64bit+.
  if (NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "'${TARGET_NAME}' requires a 64bit target arch.")
  endif()

  # At least Visual Studio 2019 version 16.8 as we use latest C++17 features.
  if (CMAKE_SYSTEM_NAME STREQUAL "Windows" AND MSVC_VERSION LESS 1928)
    message(
      FATAL_ERROR
      "This build script only supports building '${TARGET_NAME}' on 64-bit "
      "Windows with at least Visual Studio 2019 version 16.8. "
      "MSVC version '${MSVC_VERSION}' is not supported."
    )
  endif()
endfunction(verify_target_architecture)

# Alas, option() doesn't support string values.
# Use like this:
# define_strings_option(MSVC_LANGUAGE_VERSION
#   "This determines which version of C++ to compile as."
#   "c++17" "c++latest")
function(define_strings_option OPTIONS_VAR_NAME OPTIONS_VAR_DESCRIPTION DEFAULT_OPTION)
  set(ALLOWED_OPTIONS_SET "'${DEFAULT_OPTION}'")

  foreach(ALLOWED_OPTION IN LISTS ARGN)
    string(APPEND ALLOWED_OPTIONS_SET ", '${ALLOWED_OPTION}'")
  endforeach()

  set(${OPTIONS_VAR_NAME} "${DEFAULT_OPTION}" PARENT_SCOPE)
  set(${OPTIONS_VAR_NAME} "${DEFAULT_OPTION}" CACHE STRING "One of ${ALLOWED_OPTIONS_SET}. ${OPTIONS_VAR_DESCRIPTION}")
  # Add a pretty drop-down selector for these values when using the GUI.
  set_property(
    CACHE ${OPTIONS_VAR_NAME}
    PROPERTY STRINGS
      ${DEFAULT_OPTION}
      ${ARGN}
    # PARENT_SCOPE
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
endfunction(define_strings_option)

function(dump_target_property THETARGET TARGET_PROPERTY TARGET_PROPERTY_NAME)
  get_target_property(TARGET_PROPERTY_VALUE ${THETARGET} ${TARGET_PROPERTY})

  message("-- ${THETARGET} ${TARGET_PROPERTY_NAME}: ${TARGET_PROPERTY_VALUE}")

  # TODO: Encode echo argument.
  # add_custom_command(TARGET ${THETARGET}
  #  POST_BUILD
  #  COMMAND echo "${THETARGET} built with the ${TARGET_PROPERTY_NAME}: ${TARGET_PROPERTY_VALUE}")
endfunction(dump_target_property)