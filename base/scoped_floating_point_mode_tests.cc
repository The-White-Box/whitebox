// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Scoped floating point mode.  Useful when flush-to-zero and
// denormals-are-zero mode (SSE2) required for floating point operations.

#include "scoped_floating_point_mode.h"
//
#include <limits>
//
#include "base/deps/googletest/gtest/gtest.h"

#if defined(WB_ARCH_CPU_X86_64)

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(ScopedFloatingPointModeTest, SetFlushToZeroModeInScope) {
  using namespace wb::base;

  if constexpr (std::numeric_limits<float>::has_denorm == std::denorm_present) {
    // Assume no FTZ mode active.
    EXPECT_NE(_MM_GET_FLUSH_ZERO_MODE(), _MM_FLUSH_ZERO_ON)
        << "No FTZ enabled by default";

    {
      ScopedFloatingPointMode scoped_floating_point_mode{
          ScopedFloatingPointModeFlags::kFlushToZero};

      EXPECT_EQ(_MM_GET_FLUSH_ZERO_MODE(), _MM_FLUSH_ZERO_ON)
          << "FTZ is enabled in scope";
    }

    EXPECT_NE(_MM_GET_FLUSH_ZERO_MODE(), _MM_FLUSH_ZERO_ON)
        << "No FTZ enabled out of scope";
  }
}

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(ScopedFloatingPointModeTest, SetDenormalsAreZeroModeInScope) {
  using namespace wb::base;

  if constexpr (std::numeric_limits<float>::has_denorm == std::denorm_present) {
    // Assume no DAZ mode active.
    EXPECT_NE(_MM_GET_DENORMALS_ZERO_MODE(), _MM_DENORMALS_ZERO_ON)
        << "No DAZ enabled by default";

    {
      ScopedFloatingPointMode scoped_floating_point_mode{
          ScopedFloatingPointModeFlags::kDenormalsAreZero};

      EXPECT_EQ(_MM_GET_DENORMALS_ZERO_MODE(), _MM_DENORMALS_ZERO_ON)
          << "DAZ is enabled in scope";
    }

    EXPECT_NE(_MM_GET_DENORMALS_ZERO_MODE(), _MM_DENORMALS_ZERO_ON)
        << "No DAZ enabled out of scope";
  }
}

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(ScopedFloatingPointModeTest,
           SetFlushToZeroAndDenormalsAreZeroModeInScope) {
  using namespace wb::base;

  if constexpr (std::numeric_limits<float>::has_denorm == std::denorm_present) {
    // Assume no FTZ mode active.
    EXPECT_NE(_MM_GET_FLUSH_ZERO_MODE(), _MM_FLUSH_ZERO_ON)
        << "No FTZ enabled by default";
    // Assume no DAZ mode active.
    EXPECT_NE(_MM_GET_DENORMALS_ZERO_MODE(), _MM_DENORMALS_ZERO_ON)
        << "No DAZ enabled by default";

    {
      ScopedFloatingPointMode scoped_floating_point_mode{
          ScopedFloatingPointModeFlags::kFlushToZero |
          ScopedFloatingPointModeFlags::kDenormalsAreZero};

      EXPECT_EQ(_MM_GET_FLUSH_ZERO_MODE(), _MM_FLUSH_ZERO_ON)
          << "FTZ is enabled in scope";
      EXPECT_EQ(_MM_GET_DENORMALS_ZERO_MODE(), _MM_DENORMALS_ZERO_ON)
          << "DAZ is enabled in scope";
    }

    EXPECT_NE(_MM_GET_FLUSH_ZERO_MODE(), _MM_FLUSH_ZERO_ON)
        << "No FTZ enabled out of scope";
    EXPECT_NE(_MM_GET_DENORMALS_ZERO_MODE(), _MM_DENORMALS_ZERO_ON)
        << "No DAZ enabled out of scope";
  }
}

#else
#error "Please define floating point mode for your platform."
#endif
