// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Windows OS version.

#include "windows_version.h"
//
#include <sstream>
//
#include "base/deps/googletest/gtest/gtest.h"

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(WindowsVersionTest, ShouldStreamWindowsVersion) {
  std::stringstream ss;
  ss << wb::base::win::Version::WIN11_22H2;

  EXPECT_EQ("Windows 11 22H2", ss.str());

  ss.str("");

  ss << wb::base::win::Version::WIN_LAST;
  EXPECT_EQ("Unknown Windows OS", ss.str());
}

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(WindowsVersionTest, GetVersion) {
  EXPECT_NE(wb::base::win::Version::WIN_LAST, wb::base::win::GetVersion())
      << "Windows version is determinable";
}

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(WindowsVersionTest, MajorMinorBuildToVersion) {
  using namespace wb::base::win;

  for (unsigned i{0}; i <= 6U; ++i) {
    EXPECT_EQ(Version::PRE_WIN10, MajorMinorBuildToVersion(i, 0, 0))
        << "Should convert pre windows 10 OS version parts for major part "
        << i;
  }

  for (unsigned i{7}; i <= 11U; ++i) {
    if (i == 10) continue;

    EXPECT_EQ(Version::WIN_LAST, MajorMinorBuildToVersion(i, 0, 0))
        << "Should convert unknown windows 10 OS version parts for major part "
        << i;
  }

  EXPECT_EQ(Version::WIN10, MajorMinorBuildToVersion(10, 0, 0)) << "WIN10";
  EXPECT_EQ(Version::WIN10_TH2, MajorMinorBuildToVersion(10, 0, 10586U))
      << "WIN10_TH2";
  EXPECT_EQ(Version::WIN10_RS1, MajorMinorBuildToVersion(10, 0, 14393U))
      << "WIN10_RS1";
  EXPECT_EQ(Version::WIN10_RS2, MajorMinorBuildToVersion(10, 0, 15063U))
      << "WIN10_RS2";
  EXPECT_EQ(Version::WIN10_RS3, MajorMinorBuildToVersion(10, 0, 16299U))
      << "WIN10_RS3";
  EXPECT_EQ(Version::WIN10_RS4, MajorMinorBuildToVersion(10, 0, 17134U))
      << "WIN10_RS4";
  EXPECT_EQ(Version::WIN10_RS5, MajorMinorBuildToVersion(10, 0, 17763U))
      << "WIN10_RS5";
  EXPECT_EQ(Version::WIN10_19H1, MajorMinorBuildToVersion(10, 0, 18362U))
      << "WIN10_19H1";
  EXPECT_EQ(Version::WIN10_19H2, MajorMinorBuildToVersion(10, 0, 18363U))
      << "WIN10_19H2";
  EXPECT_EQ(Version::WIN10_20H1, MajorMinorBuildToVersion(10, 0, 19041U))
      << "WIN10_20H1";
  EXPECT_EQ(Version::WIN10_20H2, MajorMinorBuildToVersion(10, 0, 19042U))
      << "WIN10_20H2";
  EXPECT_EQ(Version::WIN10_21H1, MajorMinorBuildToVersion(10, 0, 19043U))
      << "WIN10_21H1";
  EXPECT_EQ(Version::WIN10_21H2, MajorMinorBuildToVersion(10, 0, 19044U))
      << "WIN10_21H2";
  EXPECT_EQ(Version::WIN11_21H2, MajorMinorBuildToVersion(10, 0, 22000U))
      << "WIN11_21H2";
  EXPECT_EQ(Version::WIN11_22H2, MajorMinorBuildToVersion(10, 0, 22621U))
      << "WIN11_22H2";
  EXPECT_EQ(Version::WIN11_23H2, MajorMinorBuildToVersion(10, 0, 22631U))
      << "WIN11_23H2";
  EXPECT_EQ(Version::WIN11_24H2, MajorMinorBuildToVersion(10, 0, 26100U))
      << "WIN11_24H2";
}
