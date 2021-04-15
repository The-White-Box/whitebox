// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Scoped handler for situations when new operator fails to allocate memory.

#ifndef WB_BASE_SCOPED_NEW_HANDLER_INTERNAL_H_
#define WB_BASE_SCOPED_NEW_HANDLER_INTERNAL_H_

#include <cstdint>

#include "base/config.h"

namespace wb::base::internals {

/**
 * @brief Get global max retries count for the new operator to reallocate
 * memory.
 * @return Global max retries count for the new operator to reallocate memory.
 */
WB_BASE_API std::uint32_t GetGlobalScopedNewHandlerMaxNewRetriesCount() noexcept;

}  // namespace wb::base::internals

#endif  // !WB_BASE_SCOPED_NEW_HANDLER_INTERNAL_H_
