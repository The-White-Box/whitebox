// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Base command line flags.

#include "base_flags.h"

#include <filesystem>

#include "base/deps/abseil/flags/flag.h"
#include "base/deps/abseil/flags/parse.h"
#include "base/deps/abseil/strings/str_cat.h"
#include "ui/static_settings_config.h"

#ifdef WB_OS_WIN
#include "base/deps/g3log/g3log.h"
#include "base/win/windows_light.h"
//
#include <timeapi.h>
#endif

namespace wb::apps::flags {

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
  if (rc == MMSYSERR_NOERROR) [[likely]] {
    if (p->ms < time_caps.wPeriodMin || p->ms > time_caps.wPeriodMax)
        [[unlikely]] {
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
               wb::ui::settings::window::dimensions::kMinWidth)};
  G3CHECK(minimum_window_width_raw <=
          static_cast<int>(std::numeric_limits<std::uint16_t>::max()));
  const std::uint16_t minimum_window_width{
      static_cast<std::uint16_t>(minimum_window_width_raw)};
#else
  const std::uint16_t minimum_window_width{
      wb::ui::settings::window::dimensions::kMinWidth};
#endif

  constexpr std::uint16_t maximum_window_width{
      std::numeric_limits<std::uint16_t>::max()};

  if (w->size < minimum_window_width || w->size > maximum_window_width)
      [[unlikely]] {
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
               wb::ui::settings::window::dimensions::kMinHeight)};
  G3CHECK(minimum_window_height_raw <=
          static_cast<int>(std::numeric_limits<std::uint16_t>::max()));
  const std::uint16_t minimum_window_height{
      static_cast<std::uint16_t>(minimum_window_height_raw)};
#else
  const std::uint16_t minimum_window_height{
      wb::ui::settings::window::dimensions::kMinHeight};
#endif

  constexpr std::uint16_t maximum_window_height{
      std::numeric_limits<std::uint16_t>::max()};

  if (h->size < minimum_window_height || h->size > maximum_window_height)
      [[unlikely]] {
    *error = absl::StrCat("not in range [", minimum_window_height, ",",
                          maximum_window_height, "]");
    return false;
  }

  return true;
}

std::string AbslUnparseFlag(AssetsPath p) {
  // Delegate to the usual unparsing for string.
  return absl::UnparseFlag(p.value);
}

bool AbslParseFlag(std::string_view text, AssetsPath* p, std::string* error) {
  // Convert from text to string_view using the string-flag parser.
  if (!absl::ParseFlag(text, &p->value, error)) {
    return false;
  }

  std::error_code rc;
  const bool has_assets_path{
      std::filesystem::exists(std::filesystem::path{p->value}, rc)};

  if (rc) [[unlikely]] {
    *error = absl::StrCat("is not valid assets path? [", rc.message(), "]");
    return false;
  }

  if (!has_assets_path) [[unlikely]] {
    *error = "assets path doesn't exist";
    return false;
  }

  return true;
}

}  // namespace wb::apps::flags

ABSL_FLAG(wb::apps::flags::AssetsPath, assets_path,
          wb::apps::flags::AssetsPath{"./"}, "assets path.");

ABSL_FLAG(std::uint32_t, attempts_to_retry_allocate_memory, 3U,
          "how many memory cleanup & reallocation attempts to do when out of "
          "memory.");

#ifdef WB_OS_WIN
ABSL_FLAG(bool, insecure_allow_unsigned_module_target, false,
          "insecure.  Allow to load NOT SIGNED module targets.  There is no "
          "guarantee unsigned module doing nothing harmful.  Use at your own "
          "risk, ex. for debugging or mods.");
#endif  // WB_OS_WIN

ABSL_FLAG(wb::apps::flags::WindowWidth, main_window_width,
          wb::apps::flags::WindowWidth{800U},
          "main window initial width in pixels.");

ABSL_FLAG(wb::apps::flags::WindowHeight, main_window_height,
          wb::apps::flags::WindowHeight{600U},
          "main window initial height in pixels.");

#ifdef WB_OS_WIN
ABSL_FLAG(
    wb::apps::flags::PeriodicTimerResolution, periodic_timer_resolution_ms,
    wb::apps::flags::PeriodicTimerResolution{8},
    "changes minimal resolution (ms) of the Windows periodic timer.  Setting a "
    "higher resolution can improve the accuracy of time-out intervals in wait "
    "functions.  However, it can also reduce overall system performance, "
    "because the thread scheduler switches tasks more often.  High resolutions "
    "can also prevent the CPU power management system from entering "
    "power-saving modes.  Setting a higher resolution does not improve the "
    "accuracy of the high-resolution performance counter.");
#endif  // WB_OS_WIN

ABSL_FLAG(bool, should_dump_heap_allocator_statistics_on_exit, false,
          "should dump heap allocator statistics on exit or not.  Included a "
          "some process info, like system/user elapsed time, peak working "
          "set size, hard page faults, etc.");
