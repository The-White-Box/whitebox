# Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
# Use of this source code is governed by a 3-Clause BSD license that can be
# found in the LICENSE file.

function(wb_check_sanitizers_configuration_valid
    THE_TARGET
    ROOT_DIRECTORY_PATH
    ENABLE_ASAN
    ENABLE_LSAN
    FORTIFY_SOURCE_DEFINED
    FORCE_ENABLE_ASAN
    ENABLE_MSAN
    ENABLE_TSAN
    ENABLE_UBSAN)
  if (${ENABLE_ASAN})
    if ("${FORTIFY_SOURCE_DEFINED}" AND NOT ${FORCE_ENABLE_ASAN})
      # ASan and -D_FORTIFY_SOURCE don't work well.  Wait till fixed in https://github.com/google/sanitizers/issues/247
      message(FATAL_ERROR
          "[sanitizers]: AddressSanitizer and source fortification are enabled.  It may cause false positives.  "
          "Please, use either AddressSanitizer or source fortification, or set force enable AddressSantizer to force one "
          "even with source fortification enabled (may lead to missed by ASan errors).")
    endif()

    if (DEFINED ENV{ASAN_OPTIONS})
      message(STATUS "${THE_TARGET}     AddressSanitizer: uses predefined ASAN_OPTIONS env variable: $ENV{ASAN_OPTIONS}")
    else()
      # Choose acceptable ASAN options as default.  Enable LSAN as default.
      set(ASAN_OPTIONS
          "strict_string_checks=1:detect_leaks=1:detect_stack_use_after_return=1:check_initialization_order=1:strict_init_order=1:suppressions=${ROOT_DIRECTORY_PATH}/sanitizers/asan_suppressions")
      set(ENV{ASAN_OPTIONS} "${ASAN_OPTIONS}")

      message(STATUS "${THE_TARGET}     AddressSanitizer: uses new ASAN_OPTIONS: $ENV{ASAN_OPTIONS}")
    endif()
  endif()

  if (${ENABLE_LSAN})
    if (DEFINED ENV{LSAN_OPTIONS})
      message(STATUS "${THE_TARGET}        LeakSanitizer: uses predefined LSAN_OPTIONS env variable: $ENV{LSAN_OPTIONS}")
    else()
      # Choose acceptable LSAN options as default.
      set(LSAN_OPTIONS "suppressions=${ROOT_DIRECTORY_PATH}/sanitizers/lsan_suppressions")
      set(ENV{LSAN_OPTIONS} "${LSAN_OPTIONS}")

      message(STATUS "${THE_TARGET}        LeakSanitizer: uses new LSAN_OPTIONS: $ENV{LSAN_OPTIONS}")
    endif()
  endif()

  if (${ENABLE_MSAN})
    if (DEFINED ENV{MSAN_OPTIONS})
      message(STATUS "${THE_TARGET}      MemorySanitizer: uses predefined MSAN_OPTIONS env variable: $ENV{MSAN_OPTIONS}")
    else()
      # Choose acceptable MSAN options as default.
      set(MSAN_OPTIONS "poison_in_dtor=1:suppressions=${ROOT_DIRECTORY_PATH}/sanitizers/msan_suppressions")
      set(ENV{MSAN_OPTIONS} "${MSAN_OPTIONS}")

      message(STATUS "${THE_TARGET}      MemorySanitizer: uses new MSAN_OPTIONS: $ENV{MSAN_OPTIONS}")
    endif()
  endif()

  if (${ENABLE_TSAN})
    if (DEFINED ENV{TSAN_OPTIONS})
      message(STATUS "${THE_TARGET}      ThreadSanitizer: uses predefined TSAN_OPTIONS env variable: $ENV{TSAN_OPTIONS}")
    else()
      # Choose acceptable TSAN options as default.
      set(TSAN_OPTIONS "suppressions=${ROOT_DIRECTORY_PATH}/sanitizers/tsan_suppressions")
      set(ENV{TSAN_OPTIONS} "${TSAN_OPTIONS}")

      message(STATUS "${THE_TARGET}      ThreadSanitizer: uses new TSAN_OPTIONS: $ENV{TSAN_OPTIONS}")
    endif()
  endif()

  if (${ENABLE_UBSAN})
    if (DEFINED ENV{UBSAN_OPTIONS})
      message(STATUS "${THE_TARGET}   UndefinedSanitizer: uses predefined UBSAN_OPTIONS env variable: $ENV{UBSAN_OPTIONS}")
    else()
      # Choose acceptable UBSAN options as default.
      set(UBSAN_OPTIONS "suppressions=${ROOT_DIRECTORY_PATH}/sanitizers/ubsan_suppressions")
      set(ENV{UBSAN_OPTIONS} "${UBSAN_OPTIONS}")

      message(STATUS "${THE_TARGET}   UndefinedSanitizer: uses new UBSAN_OPTIONS: $ENV{UBSAN_OPTIONS}")
    endif()
  endif()
endfunction()