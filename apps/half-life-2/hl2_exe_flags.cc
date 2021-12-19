// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Half-Life 2 executable command line flags.

#include "hl2_exe_flags.h"

#include "base/deps/abseil/flags/flag.h"
#include "base/deps/abseil/flags/parse.h"
#include "base/deps/abseil/strings/str_cat.h"
#include "build/static_settings_config.h"

#ifdef WB_OS_WIN
#include "base/deps/g3log/g3log.h"
#include "base/win/windows_light.h"
//
#include <timeapi.h>
#endif

namespace wb::apps::half_life_2 {

#ifdef WB_OS_WIN
std::string AbslUnparseFlag(PeriodicTimerResolution p) {
  // Delegate to the usual unparsing for int.
  return absl::UnparseFlag(p.ms);
}

bool AbslParseFlag(std::string_view text, PeriodicTimerResolution* p,
                   std::string* error) {
  // Convert from text to uint32_t using the uint32_t-flag parser.
  if (!absl::ParseFlag(text, &p->ms, error)) {
    return false;
  }

  TIMECAPS time_caps;
  const auto rc = ::timeGetDevCaps(&time_caps, sizeof(time_caps));
  if (rc == MMSYSERR_NOERROR) WB_ATTRIBUTE_LIKELY {
      if (p->ms < time_caps.wPeriodMin || p->ms > time_caps.wPeriodMax) {
        *error = absl::StrCat("not in range [", time_caps.wPeriodMin, ",",
                              time_caps.wPeriodMax, "]");
        return false;
      }

      return true;
    }

  *error =
      absl::StrCat("unable to check flag (error: ", rc, ") please, remove it");
  return false;
}
#endif  // WB_OS_WIN

std::string AbslUnparseFlag(WindowWidth w) {
  // Delegate to the usual unparsing for int.
  return absl::UnparseFlag(w.size);
}

bool AbslParseFlag(std::string_view text, WindowWidth* w, std::string* error) {
  // Convert from text to uint16_t using the uint16_t-flag parser.
  if (!absl::ParseFlag(text, &w->size, error)) {
    return false;
  }

#ifdef WB_OS_WIN
  const int minimum_window_width_raw{
      // GetSystemMetrics returns 0 on failure.
      std::max(::GetSystemMetrics(SM_CYMIN),
               wb::build::settings::ui::window::dimensions::kMinWidth)};
  G3CHECK(minimum_window_width_raw <=
          static_cast<int>(std::numeric_limits<uint16_t>::max()));
  const uint16_t minimum_window_width{
      static_cast<uint16_t>(minimum_window_width_raw)};
#else
  const uint16_t minimum_window_width{
      wb::build::settings::ui::window::dimensions::kMinWidth};
#endif

  constexpr uint16_t maximum_window_width{std::numeric_limits<uint16_t>::max()};

  if (w->size < minimum_window_width || w->size > maximum_window_width) {
    *error = absl::StrCat("not in range [", minimum_window_width, ",",
                          maximum_window_width, "]");
    return false;
  }

  return true;
}

std::string AbslUnparseFlag(WindowHeight h) {
  // Delegate to the usual unparsing for int.
  return absl::UnparseFlag(h.size);
}

bool AbslParseFlag(std::string_view text, WindowHeight* h, std::string* error) {
  // Convert from text to uint16_t using the uint16_t-flag parser.
  if (!absl::ParseFlag(text, &h->size, error)) {
    return false;
  }

#ifdef WB_OS_WIN
  const int minimum_window_height_raw{
      // GetSystemMetrics returns 0 on failure.
      std::max(::GetSystemMetrics(SM_CXMIN),
               wb::build::settings::ui::window::dimensions::kMinHeight)};
  G3CHECK(minimum_window_height_raw <=
          static_cast<int>(std::numeric_limits<uint16_t>::max()));
  const uint16_t minimum_window_height{
      static_cast<uint16_t>(minimum_window_height_raw)};
#else
  const uint16_t minimum_window_height{
      wb::build::settings::ui::window::dimensions::kMinHeight};
#endif

  constexpr uint16_t maximum_window_height{
      std::numeric_limits<uint16_t>::max()};

  if (h->size < minimum_window_height || h->size > maximum_window_height) {
    *error = absl::StrCat("not in range [", minimum_window_height, ",",
                          maximum_window_height, "]");
    return false;
  }

  return true;
}

}  // namespace wb::apps::half_life_2

ABSL_FLAG(std::uint32_t, attempts_to_retry_allocate_memory, 3U,
          "how many memory cleanup & reallocation attempts to do when out of "
          "memory.");

ABSL_FLAG(bool, should_dump_heap_allocator_statistics_on_exit, false,
          "should dump heap allocator statistics on exit or not.  Included a "
          "some process info, like system/user elapsed time, peak working "
          "set size, hard page faults, etc.");

ABSL_FLAG(wb::apps::half_life_2::WindowWidth, main_window_width,
          wb::apps::half_life_2::WindowWidth{800U},
          "main window initial width in pixels.");

ABSL_FLAG(wb::apps::half_life_2::WindowHeight, main_window_height,
          wb::apps::half_life_2::WindowHeight{600U},
          "main window initial height in pixels.");

#ifdef WB_OS_WIN
ABSL_FLAG(bool, insecure_allow_unsigned_module_target, false,
          "insecure.  Allow to load NOT SIGNED module targets.  There is no "
          "guarantee unsigned module doing nothing harmful.  Use at your own "
          "risk, ex. for debugging or mods.");

ABSL_FLAG(
    wb::apps::half_life_2::PeriodicTimerResolution,
    periodic_timer_resolution_ms,
    wb::apps::half_life_2::PeriodicTimerResolution{8},
    "changes minimal resolution (ms) of the Windows periodic timer.  Setting a "
    "higher resolution can improve the accuracy of time-out intervals in wait "
    "functions.  However, it can also reduce overall system performance, "
    "because the thread scheduler switches tasks more often.  High resolutions "
    "can also prevent the CPU power management system from entering "
    "power-saving modes.  Setting a higher resolution does not improve the "
    "accuracy of the high-resolution performance counter.");
#endif  // WB_OS_WIN