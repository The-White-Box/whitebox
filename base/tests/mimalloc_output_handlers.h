// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Mimalloc output / error handlers.

#ifndef WB_BASE_TESTS_MIMALLOC_OUTPUT_HANDLERS_H_
#define WB_BASE_TESTS_MIMALLOC_OUTPUT_HANDLERS_H_

#include "build/build_config.h"

#ifdef WB_MI_MALLOC
namespace wb::base::tests_internal {

/**
 * @brief Set mimalloc output handelrs in scope.
 */
class ScopedMimallocOutputHandlers {
 public:
  ScopedMimallocOutputHandlers();
  ~ScopedMimallocOutputHandlers();

  ScopedMimallocOutputHandlers(ScopedMimallocOutputHandlers &) = delete;
  ScopedMimallocOutputHandlers(ScopedMimallocOutputHandlers &&) = delete;
  ScopedMimallocOutputHandlers &operator=(ScopedMimallocOutputHandlers &) =
      delete;
  ScopedMimallocOutputHandlers &operator=(ScopedMimallocOutputHandlers &&) =
      delete;
};

}  // namespace wb::base::tests_internal
#endif  // WB_MI_MALLOC

#endif  // !WB_BASE_TESTS_MIMALLOC_OUTPUT_HANDLERS_H_