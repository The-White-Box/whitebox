// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Bootmgr main entry point.

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
  HINSTANCE instance;
  const char *command_line;
  const char *app_description;
  int show_window_flags;

  int main_icon_id;
  int small_icon_id;

  std::byte pad[4];
};
}  // namespace wb::bootmgr

/**
 * @brief Bootmgr entry point on Windows.
 * @param bootmgr_args Bootmgr args.
 * @return 0 on success.
 */
extern "C" [[nodiscard]] WB_BOOTMGR_API int BootmgrMain(
    const wb::bootmgr::BootmgrArgs &bootmgr_args);
#else
extern "C" WB_BOOTMGR_API int BootmgrMain(int argc, char *argv[]);
#endif