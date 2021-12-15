// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Default new allocation failure handler.

#ifndef WB_BASE_DEFAULT_NEW_HANDLER_H_
#define WB_BASE_DEFAULT_NEW_HANDLER_H_

#include "base/config.h"

namespace wb::base {

/**
 * @brief Default new memory allocation failure handler.
 * @return void.
 */
WB_BASE_API void DefaultNewFailureHandler();

}  // namespace wb::base

#endif  // !WB_BASE_DEFAULT_NEW_HANDLER_H_
