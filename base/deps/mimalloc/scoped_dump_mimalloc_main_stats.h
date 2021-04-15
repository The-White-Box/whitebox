// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Mimalloc main statistics / process info printer.

#ifndef WB_BASE_DEPS_MIMALLOC_MIMALLOC_MAIN_STATS_H_
#define WB_BASE_DEPS_MIMALLOC_MIMALLOC_MAIN_STATS_H_

#include "base/config.h"
#include "base/macroses.h"

namespace wb::mi {

/**
 * @brief Reset statistics.  Merge thread local statistics with the main
 * statistics and reset. Dumps the main mimalloc statistics.  Depends on g3log!
 */
class WB_BASE_API ScopedDumpMiMainStats {
 public:
  /**
   * @brief Reset statistics if |should_dump_stats| is true.
   * @param should_dump_stats Should dump mimalloc stats or not?
   */
  explicit ScopedDumpMiMainStats(bool should_dump_stats) noexcept;

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedDumpMiMainStats);

  /**
   * @brief If |should_dump_stats_| is true, then merge thread local statistics
   * with the main statistics and reset. Dumps the main mimalloc statistics.
   */
  ~ScopedDumpMiMainStats() noexcept;

 private:
  /**
   * @brief Should dump mimalloc stats on out of scope?
   */
  const bool should_dump_stats_;
};

}  // namespace wb::mi

#endif  // !WB_BASE_DEPS_MIMALLOC_MIMALLOC_MAIN_STATS_H_
