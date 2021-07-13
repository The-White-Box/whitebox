// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Whitebox kernel main entry point.

#ifndef WB_WHITEBOX_WHITEBOX_KERNEL_MAIN_H_
#define WB_WHITEBOX_WHITEBOX_KERNEL_MAIN_H_

#include "build/build_config.h"
#include "whitebox_kernel_api.h"

#ifdef WB_OS_WIN
#include <sal.h>

#include <cstddef>

/**
 * @brief HINSTANCE type.
 */
using HINSTANCE = struct HINSTANCE__ *;

namespace wb::kernel {
/**
 * @brief Kernel args.
 */
struct KernelArgs {
  HINSTANCE instance;
  const char *app_description;
  int show_window_flags;

  int main_icon_id;
  int small_icon_id;

  std::byte pad[4];
};
}  // namespace wb::kernel

extern "C" [[nodiscard]] WB_WHITEBOX_KERNEL_API int KernelMain(
    const wb::kernel::KernelArgs &kernel_args);
#else
extern "C" [[nodiscard]] WB_WHITEBOX_KERNEL_API int KernelMain(int argc,
                                                               char *argv[]);
#endif  // !WB_OS_WIN

#endif  // !WB_WHITEBOX_WHITEBOX_KERNEL_MAIN_H_
