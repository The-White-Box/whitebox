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
#include "base/windows/windows_light.h"

namespace wb::kernel {
WB_WHITEBOX_KERNEL_API int KernelMain(_In_ HINSTANCE instance,
                                      [[maybe_unused]] _In_ LPSTR command_line,
                                      _In_ int show_window_flags);
}  // namespace wb::kernel
#else
namespace wb::kernel {
extern "C" WB_WHITEBOX_KERNEL_API int KernelMain(int argc, char *argv[]);
}  // namespace wb::kernel
#endif  // !WB_OS_WIN

#endif  // !WB_WHITEBOX_WHITEBOX_KERNEL_MAIN_H_
