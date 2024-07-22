// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Windows OS version.
//
// Based on
// https://github.com/chromium/chromium/blob/master/base/win/windows_version.cc

#include "windows_version.h"

#include "base/deps/g3log/g3log.h"
#include "base/win/system_error_ext.h"
#include "base/win/windows_light.h"
#include "build/compiler_config.h"

namespace wb::base::win {

/**
 * @brief Gets windows version.
 * @return Windows version.
 */
[[nodiscard]] WB_BASE_API Version GetVersion() noexcept {
  static Version version{Version::WIN_LAST};

  if (version != Version::WIN_LAST) return version;

  WB_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()
    // C4996 'GetVersionExA': was declared deprecated.
    WB_MSVC_DISABLE_WARNING(4996)
    OSVERSIONINFOEX version_info = {sizeof(version_info)};
    const std::error_code rc{get_error(
        ::GetVersionEx(reinterpret_cast<OSVERSIONINFO*>(&version_info)))};
  WB_MSVC_END_WARNING_OVERRIDE_SCOPE()

  G3PCHECK_E(!rc, rc) << "Can't get Windows OS version";
  if (!rc) {
    version = MajorMinorBuildToVersion(version_info.dwMajorVersion,
                                       version_info.dwMinorVersion,
                                       version_info.dwBuildNumber);
  }

  return version;
}

/**
 * @brief Constructs version from major, minor and build components.
 * @param major Major.
 * @param minor Minor.
 * @param build Build.
 * @return Version.
 */
[[nodiscard]] WB_BASE_API wb::base::win::Version MajorMinorBuildToVersion(
    unsigned major, unsigned minor, unsigned build) noexcept {
  using wb::base::win::Version;

  // Win 10+.
  if (major == 10U) {
    // Win 11+.
    if (build >= 26100U) return Version::WIN11_24H2;
    if (build >= 22631U) return Version::WIN11_23H2;
    if (build >= 22621U) return Version::WIN11_22H2;
    if (build >= 22000U) return Version::WIN11_21H2;
    // Win 10.
    if (build >= 19045U) return Version::WIN10_22H2;
    if (build >= 19044U) return Version::WIN10_21H2;
    if (build >= 19043U) return Version::WIN10_21H1;
    if (build >= 19042U) return Version::WIN10_20H2;
    if (build >= 19041U) return Version::WIN10_20H1;
    if (build >= 18363U) return Version::WIN10_19H2;
    if (build >= 18362U) return Version::WIN10_19H1;
    if (build >= 17763U) return Version::WIN10_RS5;
    if (build >= 17134U) return Version::WIN10_RS4;
    if (build >= 16299U) return Version::WIN10_RS3;
    if (build >= 15063U) return Version::WIN10_RS2;
    if (build >= 14393U) return Version::WIN10_RS1;
    if (build >= 10586U) return Version::WIN10_TH2;
    return Version::WIN10;
  }

  // Win 8.1 or older.
  if (major <= 6U) return Version::PRE_WIN10;

  // Hitting this likely means that it's time for a >10 block above.
  G3LOG(WARNING) << "Using not supported new Windows OS version: " << major
                 << "." << minor << "." << build;
  return Version::WIN_LAST;
}

}  // namespace wb::base::win
