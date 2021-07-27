// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Command line flags.

#ifndef WB_BUILD_COMMAND_LINE_FLAGS_H_
#define WB_BUILD_COMMAND_LINE_FLAGS_H_

namespace wb::build::cmd_args {
/**
 * @brief Allow to load unsigned module targets.  Insecure!
 */
constexpr char kUnsafeAllowUnsignedModuleTargetFlag[]{
    "--unsafe-allow-unsigned-module-target"};
}  // namespace wb::build::cmd_args

#endif  // !WB_BUILD_COMMAND_LINE_FLAGS_H_
