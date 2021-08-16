// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Command line switches.

#ifndef WB_BASE_BASE_SWITCHES_H_
#define WB_BASE_BASE_SWITCHES_H_

namespace wb::base::switches {
namespace insecure {
/**
 * @brief Allow to load NOT SIGNED module targets.
 */
constexpr char kAllowUnsignedModuleTargetFlag[]{
    "--insecure-allow-unsigned-module-target"};
}  // namespace insecure
}  // namespace wb::base::switches

#endif  // !WB_BASE_BASE_SWITCHES_H_
