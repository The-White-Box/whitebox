// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Windows memory utilities.

#include "memory_utils.h"

#include "base/deps/g3log/g3log.h"
#include "base/windows/system_error_ext.h"
#include "base/windows/windows_light.h"

namespace wb::base::windows::memory {
[[nodiscard]] WB_BASE_API std::error_code
EnableTerminationOnHeapCorruption() noexcept {
  // Enables the terminate-on-corruption feature.  If the heap manager detects
  // an error in any heap used by the process, it calls the Windows Error
  // Reporting service and terminates the process.
  //
  // See
  // https://docs.microsoft.com/en-us/windows/win32/api/heapapi/nf-heapapi-heapsetinformation
  const std::error_code rc{GetErrorCode(::HeapSetInformation(
      nullptr, HeapEnableTerminationOnCorruption, nullptr, 0))};

  G3DCHECK(!rc);

  return rc;
}

/**
 * @brief Enable process termination on heap corruption.
 * @return Error code.
 */
[[nodiscard]] WB_BASE_API std::error_code
EnableHeapResourcesOptimization() noexcept {
  // If HeapSetInformation is called with HeapHandle set to NULL, then all heaps
  // in the process with a low-fragmentation heap (LFH) will have their caches
  // optimized, and the memory will be decommitted if possible.
  //
  // See
  // https://docs.microsoft.com/en-us/windows/win32/api/heapapi/nf-heapapi-heapsetinformation
  // TODO(dimhotepus): Should we call it every time when need to optimize heap?
  HEAP_OPTIMIZE_RESOURCES_INFORMATION information{
      .Version = HEAP_OPTIMIZE_RESOURCES_CURRENT_VERSION};
  const std::error_code rc{GetErrorCode(::HeapSetInformation(
      nullptr, HeapOptimizeResources, &information, sizeof(information)))};

  G3DCHECK(!rc);

  return rc;
}
}  // namespace wb::base::windows::memory
