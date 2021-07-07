// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Bootmgr main entry point.

#include "bootmgr_main.h"

#include "base/deps/g3log/g3log.h"
#include "base/deps/mimalloc/mimalloc.h"
#include "base/scoped_floating_point_mode.h"
#include "base/windows/scoped_minimum_timer_resolution.h"
#include "build/static_settings_config.h"

namespace {
/**
 * @brief Setup heap allocator.
 */
void BootHeapMemoryAllocator() {
  // Ignore earlier allocations.
  mi_stats_reset();

  G3DLOG(INFO) << "Using mi-malloc v." << mi_version();
}
}  // namespace

namespace wb::bootmgr {
/**
 * @brief Bootmgr entry point on Windows.
 * @param instance App instance.
 * @param command_line Command line.
 * @param show_window_flags Show window flags.
 * @return 0 on success.
 */
WB_BOOTMGR_API int BootmgrMain([[maybe_unused]] _In_ HINSTANCE instance,
                               [[maybe_unused]] _In_ LPSTR command_line,
                               [[maybe_unused]] _In_ int show_window_flags) {
  // Setup heap memory allocator.
  BootHeapMemoryAllocator();

  using namespace wb::base;

  // Ensure CPU floating point units convert denormals to zero and flush to zero
  // on underflow.
  const ScopedFloatingPointMode scoped_floating_point_mode{
      ScopedFloatingPointModeFlags::kDenormalsAreZero |
      ScopedFloatingPointModeFlags::kFlushToZero};

  // Set minimum timers resolution to good enough, but not too power hungry.
  const windows::ScopedMinimumTimerResolution scoped_minimum_timer_resolution{
      wb::build::settings::kMinimumTimerResolutionMs};
  G3LOG_IF(WARNING, !scoped_minimum_timer_resolution.IsSucceeded())
      << "Failed to set minimum periodic timers resolution to "
      << wb::build::settings::kMinimumTimerResolutionMs
      << "ms, will run with default system one.";

  return 0;
}
}  // namespace wb::bootmgr