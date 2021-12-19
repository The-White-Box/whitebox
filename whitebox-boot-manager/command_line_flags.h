// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Command line flags.

#ifndef WB_WHITEBOX_BOOT_MANAGER_COMMAND_LINE_FLAGS_H_
#define WB_WHITEBOX_BOOT_MANAGER_COMMAND_LINE_FLAGS_H_

#include <cstddef>  // std::byte
#include <vector>

#include "build/build_config.h"
#include "build/compiler_config.h"

namespace wb::boot_manager {

/**
 * @brief Parsed command line flags.
 */
struct CommandLineFlags {
  /**
   * @brief Flags which are not part of any flags, but unparsed ones.
   */
  std::vector<char *> positional_flags;

  /**
   * @brief How many memory cleanup & reallocation attempts to do when out of
   * memory.
   */
  std::uint32_t attempts_to_retry_allocate_memory;

#ifdef WB_OS_WIN
  /**
   * @brief Changes minimal resolution (ms) of the Windows periodic timer.
   * Setting a higher resolution can improve the accuracy of time-out intervals
   * in wait functions.  However, it can also reduce overall system performance,
   * because the thread scheduler switches tasks more often.  High resolutions
   * can also prevent the CPU power management system from entering power-saving
   * modes.  Setting a higher resolution does not improve the accuracy of the
   * high-resolution performance counter.
   */
  std::uint32_t periodic_timer_resolution_ms;
#endif

  /**
   * @brief Initial width of the main window in pixels.
   */
  std::uint16_t main_window_width;

  /**
   * @brief Initial height of the main window in pixels.
   */
  std::uint16_t main_window_height;

  /**
   * @brief Insecure.  Allow to load NOT SIGNED module targets.  There is no
   * guarantee unsigned module doing nothing harmful.  Use at your own risk, ex.
   * for debugging or mods.
   */
  bool insecure_allow_unsigned_module_target;

  /**
   * @brief Should dump heap allocator statistics on exit or not.  Included a
   * bit of process info, like system/user elapsed time, peak working set size,
   * hard page faults, etc.
   */
  bool should_dump_heap_allocator_statistics_on_exit;

  WB_ATTRIBUTE_UNUSED_FIELD std::byte
      pad_[sizeof(uint32_t) - sizeof(insecure_allow_unsigned_module_target) -
           sizeof(should_dump_heap_allocator_statistics_on_exit)] = {};
};

}  // namespace wb::boot_manager

#endif  // !WB_WHITEBOX_BOOT_MANAGER_COMMAND_LINE_FLAGS_H_