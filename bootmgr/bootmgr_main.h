// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Bootmgr main entry point.

#ifndef WB_BOOTMGR_BOOTMGR_MAIN_H_
#define WB_BOOTMGR_BOOTMGR_MAIN_H_

#include "bootmgr_api.h"
#include "build/build_config.h"

#ifdef WB_OS_WIN
#include "base/windows/windows_light.h"

namespace wb::bootmgr {
/**
 * @brief Bootmgr entry point on Windows.
 * @param instance App instance.
 * @param command_line Command line.
 * @param show_window_flags Show window flags.
 * @return 0 on success.
 */
WB_BOOTMGR_API int BootmgrMain(_In_ HINSTANCE instance,
                               [[maybe_unused]] _In_ LPSTR command_line,
                               _In_ int show_window_flags);
}  // namespace wb::bootmgr
#else
namespace wb::bootmgr {
extern "C" WB_BOOTMGR_API int BootmgrMain(int argc, char *argv[]);
}  // namespace wb::bootmgr
#endif  // !WB_OS_WIN

#endif  // !WB_BOOTMGR_BOOTMGR_MAIN_H_
