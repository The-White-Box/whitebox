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

#include <ostream>

#include "base/config.h"
#include "base/deps/g3log/g3log.h"

namespace wb::base::win {

/**
 * @brief The running version of Windows.  NOTE: Keep these in order so callers
 * can do things like
 * "if (base::win::GetVersion() >= base::win::Version::WIN10_TH2) ...".
 */
enum class Version {
  PRE_WIN10 = 0,  // Not supported.

  WIN10 = 7,        // Windows 10. Threshold 1: Version 1507, Build 10240.
  WIN10_TH2 = 8,    // Windows 10. Threshold 2: Version 1511, Build 10586.
  WIN10_RS1 = 9,    // Windows 10. Redstone 1: Version 1607, Build 14393.
  WIN10_RS2 = 10,   // Windows 10. Redstone 2: Version 1703, Build 15063.
  WIN10_RS3 = 11,   // Windows 10. Redstone 3: Version 1709, Build 16299.
  WIN10_RS4 = 12,   // Windows 10. Redstone 4: Version 1803, Build 17134.
  WIN10_RS5 = 13,   // Windows 10. Redstone 5: Version 1809, Build 17763.
  WIN10_19H1 = 14,  // Windows 10. 19H1: Version 1903, Build 18362.
  WIN10_19H2 = 15,  // Windows 10. 19H2: Version 1909, Build 18363.
  WIN10_20H1 = 16,  // Windows 10. 20H1: Build 19041.
  WIN10_20H2 = 17,  // Windows 10. 20H2: Build 19042.
  WIN10_21H1 = 18,  // Windows 10. 21H1: Build 19043.
  WIN10_21H2 = 19,  // Windows 10. 21H2: Build 19044.

  WIN11_21H2 = 20,  // Windows 11. 21H2: Build 22000.
  WIN11_22H2 = 21,  // Windows 11. 22H2: Build 22621.
  WIN_LAST,         // Indicates error condition.
};

/**
 * @brief Allows to use operator << for streaming Version.
 * @param s Stream.
 * @param version Version.
 * @return Stream.
 */
inline auto& operator<<(std::basic_ostream<char, std::char_traits<char>>& s,
                        Version version) {
  using namespace wb::base::win;

  switch (version) {
    case Version::PRE_WIN10:
      return s << "Pre Windows 10";
    case Version::WIN10:
      return s << "Windows 10 Threshold 1: Version 1507";
    case Version::WIN10_TH2:
      return s << "Windows 10 Threshold 2: Version 1511";
    case Version::WIN10_RS1:
      return s << "Windows 10 Redstone 1: Version 1607";
    case Version::WIN10_RS2:
      return s << "Windows 10 Redstone 2: Version 1703";
    case Version::WIN10_RS3:
      return s << "Windows 10 Redstone 3: Version 1709";
    case Version::WIN10_RS4:
      return s << "Windows 10 Redstone 4: Version 1803";
    case Version::WIN10_RS5:
      return s << "Windows 10 Redstone 5: Version 1809";
    case Version::WIN10_19H1:
      return s << "Windows 10 19H1: Version 1903";
    case Version::WIN10_19H2:
      return s << "Windows 10 19H2: Version 1909";
    case Version::WIN10_20H1:
      return s << "Windows 10 20H1";
    case Version::WIN10_20H2:
      return s << "Windows 10 20H2";
    case Version::WIN10_21H1:
      return s << "Windows 10 21H1";
    case Version::WIN10_21H2:
      return s << "Windows 10 21H2";
    case Version::WIN11_21H2:
      return s << "Windows 11 21H2";
    case Version::WIN11_22H2:
      return s << "Windows 11 22H2";
    case Version::WIN_LAST:
      return s << "Unknown Windows OS";
    default:
      G3DCHECK(version > Version::WIN_LAST)
          << "Missed Version value in stream operator.";
      return s << "Unknown Windows OS";
  }
}

/**
 * @brief Gets Windows OS version.
 * @return Windows version.
 */
[[nodiscard]] WB_BASE_API Version GetVersion() noexcept;

/**
 * @brief Constructs version from major, minor and build components.
 * @param major Major.
 * @param minor Minor.
 * @param build Build.
 * @return Version.
 */
[[nodiscard]] WB_BASE_API Version MajorMinorBuildToVersion(
    unsigned major, unsigned minor, unsigned build) noexcept;

}  // namespace wb::base::win

#endif  // !WB_BASE_WIN_WINDOWS_VERSION_H_
