# Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
# Use of this source code is governed by a 3-Clause BSD license that can be
# found in the LICENSE file.
#
# Half-Life 2 launcher project definition.

cmake_minimum_required (VERSION 3.9 FATAL_ERROR)

project("Half-Life 2"
  VERSION ${CMAKE_PROJECT_VERSION}
  DESCRIPTION "Half-Life 2"
  LANGUAGES CXX)

set(WB_HALF_LIFE2_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(WB_HALF_LIFE2_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(WB_HALF_LIFE2_TARGET_NAME "half-life-2")
set(WB_CURRENT_TARGET_NAME ${WB_HALF_LIFE2_TARGET_NAME})
set(WB_CURRENT_TARGET_SUFFIX ".exe")

# We currently build app by finding all sources and header files.
# We then exclude specific files below.
wb_auto_sources(files "*.cc" "RECURSE" "${WB_HALF_LIFE2_SOURCE_DIR}")
wb_auto_sources(hfiles "*.h" "RECURSE" "${WB_HALF_LIFE2_SOURCE_DIR}")

# Generate project info.
configure_file(
  ${WB_ROOT_DIR}/build/app_version_config.h.cmake.in
  ${WB_HALF_LIFE2_BINARY_DIR}/gen/app_version_config.h
)

if (WB_OS_WINDOWS)
  add_executable(${WB_HALF_LIFE2_TARGET_NAME} WIN32 ${files} ${hfiles})
elseif (WB_OS_MACOSX)
  add_executable(${WB_HALF_LIFE2_TARGET_NAME} MACOSX_BUNDLE ${files} ${hfiles})
else()
  add_executable(${WB_HALF_LIFE2_TARGET_NAME} ${files} ${hfiles})
endif()

# Include the directory with the new files.
target_include_directories(${WB_HALF_LIFE2_TARGET_NAME}
  PRIVATE
    ${WB_ROOT_DIR}
    ${WB_HALF_LIFE2_BINARY_DIR}/gen
    ${WB_BINARY_DIR}
)

# Append Windows specific.
if (WB_OS_WINDOWS)
  set(WB_OS_WINDOWS_MANIFESTS_DIR "${WB_ROOT_DIR}/build/windows/manifests")
  target_sources(${WB_HALF_LIFE2_TARGET_NAME}
    PRIVATE
      windows_half_life_2.rc
      ${WB_OS_WINDOWS_MANIFESTS_DIR}/dpi-aware.manifest
      ${WB_BINARY_DIR}/enable-visual-styles.manifest
      ${WB_OS_WINDOWS_MANIFESTS_DIR}/supported-os.manifest
      ${WB_OS_WINDOWS_MANIFESTS_DIR}/ultra-high-scroll-resolution.manifest
      ${WB_OS_WINDOWS_MANIFESTS_DIR}/utf-8-code-page.manifest
  )

  # Generate visual styles manifest as requires project info.
  configure_file(
    ${WB_OS_WINDOWS_MANIFESTS_DIR}/enable-visual-styles.manifest.cmake
    ${WB_BINARY_DIR}/enable-visual-styles.manifest
  )
endif()

# Specify project compile / link options.
wb_apply_compile_options_to_target(${WB_HALF_LIFE2_TARGET_NAME})

set(WB_HALF_LIFE2_DEPENDENT_LINK_LIBRARIES base bootmgr g3log mimalloc whitebox-kernel)
set(WB_HALF_LIFE2_OS_SPECIFIC_LINK_LIBRARIES "comsuppw")

target_link_libraries(${WB_HALF_LIFE2_TARGET_NAME}
  ${WB_HALF_LIFE2_DEPENDENT_LINK_LIBRARIES}
  ${WB_HALF_LIFE2_OS_SPECIFIC_LINK_LIBRARIES}
)

# Add runtime dependencies.
wb_copy_all_target_dependencies_to_target_bin_dir(${WB_HALF_LIFE2_TARGET_NAME}
  "${WB_HALF_LIFE2_DEPENDENT_LINK_LIBRARIES}")
