// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Heap allocator boot.

#ifndef WB_APPS_BOOT_HEAP_ALLOCATOR_H_
#define WB_APPS_BOOT_HEAP_ALLOCATOR_H_

#include "build/compiler_config.h"

namespace wb::apps {

/**
 * @brief Setup heap allocator.
 */
void BootHeapAllocator() noexcept;

}  // namespace wb::apps

#endif  // !WB_APPS_BOOT_HEAP_ALLOCATOR_H_
