// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// G3log console sink.

#ifndef WB_BASE_DEPS_G3LOG_CONSOLE_SINK_H_
#define WB_BASE_DEPS_G3LOG_CONSOLE_SINK_H_

#include <iostream>

#include "base/base_macroses.h"
#include "build/build_config.h"
#include "g3log_config.h"

WB_BEGIN_G3LOG_WARNING_OVERRIDE_SCOPE()
#include "deps/g3log/src/g3log/logmessage.hpp"
WB_END_G3LOG_WARNING_OVERRIDE_SCOPE()

#ifdef WB_OS_WIN
#include "base/win/windows_light.h"
#endif

namespace wb::base::deps::g3log {
/**
 * @brief Console sink.  Useful for debugging.
 */
struct ConsoleSink {
#ifdef WB_OS_POSIX
  // Linux xterm color.
  // https://stackoverflow.com/questions/2616906/how-do-i-output-coloured-text-to-a-linux-terminal
  enum class ForegroundColor : int {
    kYellow = 33,
    kRed = 31,
    kGreen = 32,
    kDefault = 39
  };

  ForegroundColor GetColor(const LEVELS level) const {
    if (level.value == WARNING.value) {
      return ForegroundColor::kYellow;
    }
    if (level.value == DBUG.value) {
      return ForegroundColor::kGreen;
    }
    if (g3::internal::wasFatal(level)) {
      return ForegroundColor::kRed;
    }

    return ForegroundColor::kDefault;
  }
#endif

  void ReceiveLogMessage(g3::LogMessageMover logEntry) const {
#ifdef WB_OS_POSIX
    const auto level = logEntry.get()._level;
    const auto color = GetColor(level);

    std::cerr << "\033[" << wb::base::underlying_cast(color) << "m"
              << logEntry.get().toString() << "\033[m" << std::endl;
#elif defined(WB_OS_WIN)
    const auto message = logEntry.get().toString();
    ::OutputDebugStringA(message.c_str());
#else
#error Please define console output sink for your platform.
#endif
  }
};
}  // namespace wb::base::deps::g3log

#endif  // !WB_BASE_DEPS_G3LOG_CONSOLE_SINK_H_
