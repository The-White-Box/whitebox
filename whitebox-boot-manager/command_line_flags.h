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
  uint32_t periodic_timer_resolution_ms;
#endif

  /**
   * @brief Insecure.  Allow to load NOT SIGNED module targets.  There is no
   * guarantee unsigned module doing nothing harmful.  Use at your own risk, ex.
   * for debugging or mods.
   */
  bool insecure_allow_unsigned_module_target;

  WB_ATTRIBUTE_UNUSED_FIELD std::byte pad_[
#ifdef WB_OS_WIN
      sizeof(periodic_timer_resolution_ms) -
#else
      sizeof(char *) -
#endif
      sizeof(insecure_allow_unsigned_module_target)] = {};
};

}  // namespace wb::boot_manager

#endif  // !WB_WHITEBOX_BOOT_MANAGER_COMMAND_LINE_FLAGS_H_