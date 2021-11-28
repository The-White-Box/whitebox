// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Half-Life 2 executable command line flags.

#include "hl2_exe_flags.h"

#include "base/deps/abseil/flags/declare.h"
#include "base/deps/abseil/flags/flag.h"
#include "base/deps/abseil/flags/parse.h"
#include "base/deps/abseil/strings/str_cat.h"

#ifdef WB_OS_WIN
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

}  // namespace wb::apps::half_life_2

#ifdef WB_OS_WIN
ABSL_FLAG(bool, insecure_allow_unsigned_module_target, false,
          "Insecure.  Allow to load NOT SIGNED module targets.  There is no "
          "guarantee unsigned module doing nothing harmful.  Use at your own "
          "risk, ex. for debugging or mods.");

ABSL_FLAG(
    wb::apps::half_life_2::PeriodicTimerResolution,
    periodic_timer_resolution_ms,
    wb::apps::half_life_2::PeriodicTimerResolution{8},
    "Changes minimal resolution (ms) of the Windows periodic timer.  Setting a "
    "higher resolution can improve the accuracy of time-out intervals in wait "
    "functions.  However, it can also reduce overall system performance, "
    "because the thread scheduler switches tasks more often.  High resolutions "
    "can also prevent the CPU power management system from entering "
    "power-saving modes.  Setting a higher resolution does not improve the "
    "accuracy of the high-resolution performance counter.");
#endif  // WB_OS_WIN