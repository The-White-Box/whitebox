// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// posix <system_error> extensions.

#ifndef WB_BASE_POSIX_SYSTEM_ERROR_EXT_H_
#define WB_BASE_POSIX_SYSTEM_ERROR_EXT_H_

#include "base/std2/system_error_ext.h"

namespace wb::base::posix {

/**
 * Get error code by return result.
 * @param result API return result.
 * @return std::error_code.
 */
[[nodiscard]] inline std::error_code get_error(int result) noexcept {
  return !result ? std2::ok_code : std2::system_last_error_code();
}

}  // namespace wb::base::posix

#endif  // !WB_BASE_POSIX_SYSTEM_ERROR_EXT_H_
