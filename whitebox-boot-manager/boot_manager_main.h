// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Boot manager main entry point.

#ifndef WB_WHITEBOX_BOOT_MANAGER_BOOT_MANAGER_MAIN_H_
#define WB_WHITEBOX_BOOT_MANAGER_BOOT_MANAGER_MAIN_H_

#include <cstddef>  // std::byte

#include "base/deps/g3log/g3log.h"
#include "base/intl/lookup.h"
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
 * @brief Boot manager args.
 */
struct BootmgrArgs {
#ifdef WB_OS_WIN
  BootmgrArgs(HINSTANCE instance_, const char *command_line_,
              const char *app_description_, int show_window_flags_,
              int main_icon_id_, int small_icon_id_,
              const wb::base::intl::LookupWithFallback &intl_)
      : instance{instance_},
        command_line{command_line_},
        app_description{app_description_},
        show_window_flags{show_window_flags_},
        main_icon_id{main_icon_id_},
        small_icon_id{small_icon_id_},
        intl{intl_} {
    G3DCHECK(!!instance);
    G3DCHECK(!!command_line);
    G3DCHECK(!!app_description);
  }
#else
  BootmgrArgs(const char *app_description_, char **const argv_, const int argc_,
              const wb::base::intl::LookupWithFallback &intl_)
      : app_description{app_description_},
        argv{argv_},
        argc{argc_},
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
  /**
   * @brief Command line.
   */
  const char *command_line;
#endif

  /**
   * @brief App description.
   */
  const char *app_description;

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
  /**
   * @brief App arguments.
   */
  char **const argv;
  /**
   * @brief App arguments count.
   */
  const int argc;
#endif

  /**
   * @brief Align to machine word boundary.
   */
  std::byte pad[4];

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