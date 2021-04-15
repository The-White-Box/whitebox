// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Mimalloc main statistics / process info printer.

#include "base/deps/mimalloc/scoped_dump_mimalloc_main_stats.h"

#include "base/deps/g3log/g3log.h"
#include "base/deps/mimalloc/mimalloc.h"
#include "base/std2/string_view_ext.h"

namespace {

/**
 * @brief Print mimalloc stats.
 * @param msg Stats to print.
 * @param arg Optional state.
 * @return void.
 */
void OutMiStats(const char *msg, [[maybe_unused]] void *arg) noexcept {
  std::string_view message{msg};

  if (wb::base::std2::ends_with(message, '\n')) {
    message = std::move(message.substr(0, message.size() - 1));
  }

  G3LOG(G3LOG_DEBUG) << message;
}

/**
 * @brief Print mimalloc stats.
 * @return void.
 */
void PrintMiStats() noexcept {
  G3LOG(G3LOG_DEBUG) << "Mi-malloc stats: ";

  ::mi_stats_print_out(OutMiStats, nullptr);
}

}  // namespace

namespace wb::mi {

ScopedDumpMiMainStats::ScopedDumpMiMainStats(bool should_dump_stats) noexcept
    : should_dump_stats_{should_dump_stats} {
  if (should_dump_stats) ::mi_stats_reset();
}

ScopedDumpMiMainStats::~ScopedDumpMiMainStats() noexcept {
  if (should_dump_stats_) {
    // Merge thread local statistics with the main statistics and reset.
    ::mi_stats_merge();

    // Print the main statistics (and process statistics).  Most detailed when
    // using a debug build.
    PrintMiStats();
  }
}

}  // namespace wb::mi
