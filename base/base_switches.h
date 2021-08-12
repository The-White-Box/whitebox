// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Command line switches.

#ifndef WB_BASE_BASE_SWITCHES_H_
#define WB_BASE_BASE_SWITCHES_H_

namespace wb::base::switches {
/**
 * @brief Allow to load unsigned module targets.  Insecure!
 */
constexpr char kUnsafeAllowUnsignedModuleTargetFlag[]{
    "--unsafe-allow-unsigned-module-target"};
}  // namespace wb::base::switches

#endif  // !WB_BASE_BASE_SWITCHES_H_
