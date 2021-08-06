// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Bootmgr main entry point.

#include "base/deps/g3log/g3log.h"
#include "bootmgr/bootmgr_api.h"
#include "build/build_config.h"

#ifdef WB_OS_WIN
#include <sal.h>

#include <cstddef>

/**
 * @brief HINSTANCE type.
 */
using HINSTANCE = struct HINSTANCE__ *;

namespace wb::bootmgr {
/**
 * @brief Boot manager args.
 */
struct BootmgrArgs {
  /**
   * @brief App instance.
   */
  HINSTANCE instance;
  /**
   * @brief Command line.
   */
  const char *command_line;
  /**
   * @brief App description.
   */
  const char *app_description;
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

  /**
   * @brief Align to machine word boundary.
   */
  std::byte pad[4];
};
}  // namespace wb::bootmgr

/**
 * @brief Boot manager entry point on Windows.
 * @param bootmgr_args Boot manager args.
 * @return 0 on success.
 */
extern "C" [[nodiscard]] WB_BOOTMGR_API int BootmgrMain(
    const wb::bootmgr::BootmgrArgs &bootmgr_args);
#else

namespace wb::bootmgr {
/**
 * @brief Bootmgr args.
 */
struct BootmgrArgs {
  BootmgrArgs(const char *appDescription, char **const argv_, const int argc_)
      : app_description{appDescription}, argv{argv_}, argc{argc_} {
    G3DCHECK(!!appDescription);
    G3DCHECK(!!argc_);
    G3DCHECK(!!argv_);
  }

  /**
   * @brief App description.
   */
  const char *app_description;
  /**
   * App arguments.
   */
  char **const argv;
  /**
   * App arguments count.
   */
  const int argc;

  std::byte pad[4]{};
};
}  // namespace wb::bootmgr

/**
 * @brief Boot manager entry point on *nix and mac.
 * @param argc Arguments count.
 * @param argv Arguments.
 * @return 0 on success.
 */
extern "C" [[nodiscard]] WB_BOOTMGR_API int BootmgrMain(
    const wb::bootmgr::BootmgrArgs &bootmgr_args);
#endif