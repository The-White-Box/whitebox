// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Local memory scoper.

#ifndef WB_BASE_WIN_MEMORY_SCOPED_LOCAL_MEMORY_H_
#define WB_BASE_WIN_MEMORY_SCOPED_LOCAL_MEMORY_H_

#include <sal.h>
#include <specstrings.h>  // _Frees_ptr_opt_

#include "base/base_macroses.h"
#include "build/compiler_config.h"

using HANDLE = void*;
using HLOCAL = HANDLE;

extern "C" WB_ATTRIBUTE_DLL_IMPORT _Success_(return == 0) _Ret_maybenull_ HLOCAL
    __stdcall LocalFree(_Frees_ptr_opt_ HLOCAL memory);

namespace wb::base::windows::memory {

/**
 * @brief Scoped local memory holder.
 */
class ScopedLocalMemory {
 public:
  /**
   * @brief Creates scope for local memory handle.  Frees handle out of scope.
   * @param owning_memory Memory handle to become owner of.
   */
  explicit ScopedLocalMemory(_In_ HLOCAL owning_memory) noexcept
      : memory_{owning_memory} {}

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedLocalMemory);

  /**
   * @brief Releases local memory.
   */
  ~ScopedLocalMemory() noexcept { G3CHECK(::LocalFree(memory_) == nullptr); }

 private:
  /**
   * @brief Memory handle.
   */
  const HLOCAL memory_;
};

}  // namespace wb::base::windows::memory

#endif  // WB_BASE_WIN_MEMORY_SCOPED_LOCAL_MEMORY_H_