// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Boot manager main entry point.

#ifndef WB_WHITEBOX_BOOT_MANAGER_BOOT_MANAGER_MAIN_H_
#define WB_WHITEBOX_BOOT_MANAGER_BOOT_MANAGER_MAIN_H_

#include <cstddef>  // std::byte
#include <vector>

#include "base/deps/g3log/g3log.h"
#include "base/intl/lookup_with_fallback.h"
#include "build/build_config.h"
#include "whitebox-boot-manager/api.h"

#ifdef WB_OS_WIN
/**
 * @brief HINSTANCE type.
 */
using HINSTANCE = struct HINSTANCE__ *;
#endif

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

/**
 * @brief Boot manager args.
 */
struct BootmgrArgs {
#ifdef WB_OS_WIN
  BootmgrArgs(HINSTANCE instance_, char **const argv_, const int argc_,
              const char *app_description_, int show_window_flags_,
              int main_icon_id_, int small_icon_id_,
              const CommandLineFlags &command_line_flags_,
              const wb::base::intl::LookupWithFallback &intl_)
      : instance{instance_},
        app_description{app_description_},
        argv{argv_},
        argc{argc_},
        show_window_flags{show_window_flags_},
        main_icon_id{main_icon_id_},
        small_icon_id{small_icon_id_},
        command_line_flags{command_line_flags_},
        intl{intl_} {
    G3DCHECK(!!instance);
    G3DCHECK(!!app_description);
    G3DCHECK(!!argc_);
    G3DCHECK(!!argv_);
  }
#else
  BootmgrArgs(const char *app_description_, char **const argv_, const int argc_,
              const CommandLineFlags &command_line_flags_,
              const wb::base::intl::LookupWithFallback &intl_)
      : app_description{app_description_},
        argv{argv_},
        argc{argc_},
        command_line_flags{command_line_flags_},
        intl{intl_} {
    G3DCHECK(!!app_description_);
    G3DCHECK(!!argc_);
    G3DCHECK(!!argv_);
  }
#endif

#ifdef WB_OS_WIN
  /**
   * @brief App instance.
   */
  HINSTANCE instance;
#endif

  /**
   * @brief App description.
   */
  const char *app_description;
  /**
   * @brief App arguments.
   */
  char **const argv;
  /**
   * @brief App arguments count.
   */
  const int argc;

#ifdef WB_OS_WIN
  /**
   * @brief Show app window flags.
   */
  int show_window_flags;

  /**
   * @brief Main app icon id.
   */
  int main_icon_id;
  /**
   * @brief Small app icon id.
   */
  int small_icon_id;
#else
  WB_ATTRIBUTE_UNUSED_FIELD std::byte pad_[sizeof(char *) - sizeof(argc)];
#endif

  /**
   * @brief Command line flags.
   */
  const CommandLineFlags &command_line_flags;

  /**
   * @brief Localization service.
   */
  const wb::base::intl::LookupWithFallback &intl;

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(BootmgrArgs);
};

}  // namespace wb::boot_manager

/**
 * @brief Boot manager entry point on Windows.
 * @param bootmgr_args Boot manager args.
 * @return 0 on success.
 */
extern "C" [[nodiscard]] WB_BOOT_MANAGER_API int BootmgrMain(
    const wb::boot_manager::BootmgrArgs &bootmgr_args);

#endif  // !WB_WHITEBOX_BOOT_MANAGER_BOOT_MANAGER_MAIN_H_