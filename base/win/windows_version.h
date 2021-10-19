// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Windows OS version.
//
// Based on
// https://github.com/chromium/chromium/blob/master/base/win/windows_version.h

#ifndef WB_BASE_WIN_WINDOWS_VERSION_H_
#define WB_BASE_WIN_WINDOWS_VERSION_H_

#include "base/base_api.h"

namespace wb::base::win {

/**
 * @brief The running version of Windows.  NOTE: Keep these in order so callers
 * can do things like
 * "if (base::win::GetVersion() >= base::win::Version::WIN10_TH2) ...".
 */
enum class Version {
  PRE_WIN10 = 0,    // Not supported.
  WIN10 = 7,        // Threshold 1: Version 1507, Build 10240.
  WIN10_TH2 = 8,    // Threshold 2: Version 1511, Build 10586.
  WIN10_RS1 = 9,    // Redstone 1: Version 1607, Build 14393.
  WIN10_RS2 = 10,   // Redstone 2: Version 1703, Build 15063.
  WIN10_RS3 = 11,   // Redstone 3: Version 1709, Build 16299.
  WIN10_RS4 = 12,   // Redstone 4: Version 1803, Build 17134.
  WIN10_RS5 = 13,   // Redstone 5: Version 1809, Build 17763.
  WIN10_19H1 = 14,  // 19H1: Version 1903, Build 18362.
  WIN10_19H2 = 15,  // 19H2: Version 1909, Build 18363.
  WIN10_20H1 = 16,  // 20H1: Build 19041.
  WIN10_20H2 = 17,  // 20H2: Build 19042.
  WIN10_21H1 = 18,  // 21H1: Build 19043.
  WIN_LAST,         // Indicates error condition.
};

/**
 * @brief Gets Windows OS version.
 * @return Windows version.
 */
[[nodiscard]] WB_BASE_API Version GetVersion() noexcept;

}  // namespace wb::base::win

#endif  // !WB_BASE_WIN_WINDOWS_VERSION_H_
