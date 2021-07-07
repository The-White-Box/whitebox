// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Whiotebox kernel main entry point.

#include "whitebox_kernel_main.h"

namespace wb::kernel {
WB_WHITEBOX_KERNEL_API int KernelMain(
    [[maybe_unused]] _In_ HINSTANCE instance,
    [[maybe_unused]] _In_ LPSTR command_line,
    [[maybe_unused]] _In_ int show_window_flags) {
  return 0;
}
}  // namespace wb::kernel