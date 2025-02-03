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

#if defined(WB_ARCH_CPU_X86_64) || defined(WB_ARCH_CPU_ARM_FAMILY) || \
    defined(WB_ARCH_CPU_ARM_NEON)

namespace {

constexpr inline wb::base::floating_point_register_t kFlushToZeroOn =
    std::to_underlying(wb::base::ScopedFloatFlushToZeroFlags::kFlushToZeroOn);

[[nodiscard]] wb::base::floating_point_register_t
GetFlushToZeroMode() noexcept {
  return wb::base::GetFloatingPointStatusRegister() & kFlushToZeroOn;
}

constexpr inline wb::base::floating_point_register_t kDenormalsAreZeroOn =
    std::to_underlying(
        wb::base::ScopedFloatDenormalsAreZeroFlags::kDenormalsAreZeroOn);

[[nodiscard]] wb::base::floating_point_register_t
GetDenormalsAreZeroMode() noexcept {
  return wb::base::GetFloatingPointStatusRegister() & kDenormalsAreZeroOn;
}

}  // namespace

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(ScopedFloatingPointModeTest, SetFlushToZeroModeInScope) {
  using namespace wb::base;

  // Assume no FTZ mode active.
  EXPECT_NE(GetFlushToZeroMode(), kFlushToZeroOn)
      << "No FTZ enabled by default";

  {
    ScopedFloatFlushToZeroMode scoped_float_ftz_mode{
        ScopedFloatFlushToZeroFlags::kFlushToZeroOn};

    EXPECT_EQ(GetFlushToZeroMode(), kFlushToZeroOn)
        << "FTZ is enabled in scope 1";
  }

  EXPECT_NE(GetFlushToZeroMode(), kFlushToZeroOn)
      << "No FTZ enabled out of scope 1";

  {
    ScopedFloatFlushToZeroMode scoped_float_ftz_mode1{
        ScopedFloatFlushToZeroFlags::kFlushToZeroOn};

    EXPECT_EQ(GetFlushToZeroMode(), kFlushToZeroOn)
        << "FTZ is enabled in scope 2";

    ScopedFloatFlushToZeroMode scoped_float_ftz_mode2{
        ScopedFloatFlushToZeroFlags::kFlushToZeroOff};

    EXPECT_NE(GetFlushToZeroMode(), kFlushToZeroOn)
        << "FTZ is disabled in scope 2";
  }

  EXPECT_NE(GetFlushToZeroMode(), kFlushToZeroOn)
      << "No FTZ enabled out of scope 2";
}

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(ScopedFloatingPointModeTest, SetDenormalsAreZeroModeInScope) {
  using namespace wb::base;

  // Assume no DAZ mode active.
  EXPECT_NE(GetDenormalsAreZeroMode(),
            static_cast<unsigned>(kDenormalsAreZeroOn))
      << "No DAZ enabled by default";

  {
    ScopedFloatDenormalsAreZeroMode scoped_float_daz_mode{
        ScopedFloatDenormalsAreZeroFlags::kDenormalsAreZeroOn};

    EXPECT_EQ(GetDenormalsAreZeroMode(),
              static_cast<unsigned>(kDenormalsAreZeroOn))
        << "DAZ is enabled in scope 1";
  }

  EXPECT_NE(GetDenormalsAreZeroMode(),
            static_cast<unsigned>(kDenormalsAreZeroOn))
      << "No DAZ enabled out of scope 1";

  {
    ScopedFloatDenormalsAreZeroMode scoped_float_daz_mode1{
        ScopedFloatDenormalsAreZeroFlags::kDenormalsAreZeroOn};

    EXPECT_EQ(GetDenormalsAreZeroMode(),
              static_cast<unsigned>(kDenormalsAreZeroOn))
        << "DAZ is enabled in scope 2";

    ScopedFloatDenormalsAreZeroMode scoped_float_daz_mode2{
        ScopedFloatDenormalsAreZeroFlags::kDenormalsAreZeroOff};

    EXPECT_NE(GetDenormalsAreZeroMode(),
              static_cast<unsigned>(kDenormalsAreZeroOn))
        << "DAZ is disabled in scope 2";
  }

  EXPECT_NE(GetDenormalsAreZeroMode(),
            static_cast<unsigned>(kDenormalsAreZeroOn))
      << "No DAZ enabled out of scope 2";
}

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(ScopedFloatingPointModeTest,
           SetFlushToZeroAndDenormalsAreZeroModeInScope) {
  using namespace wb::base;

  // Assume no FTZ mode active.
  EXPECT_NE(GetFlushToZeroMode(), kFlushToZeroOn)
      << "No FTZ enabled by default";
  // Assume no DAZ mode active.
  EXPECT_NE(GetDenormalsAreZeroMode(),
            static_cast<unsigned>(kDenormalsAreZeroOn))
      << "No DAZ enabled by default";

  {
    ScopedFloatFlushToZeroMode scoped_float_ftz_mode{
        ScopedFloatFlushToZeroFlags::kFlushToZeroOn};

    EXPECT_EQ(GetFlushToZeroMode(), kFlushToZeroOn)
        << "FTZ is enabled in scope 1";

    ScopedFloatDenormalsAreZeroMode scoped_float_daz_mode{
        ScopedFloatDenormalsAreZeroFlags::kDenormalsAreZeroOn};

    EXPECT_EQ(GetDenormalsAreZeroMode(),
              static_cast<unsigned>(kDenormalsAreZeroOn))
        << "DAZ is enabled in scope 1";
  }

  EXPECT_NE(GetFlushToZeroMode(), kFlushToZeroOn)
      << "No FTZ enabled out of scope 1";
  EXPECT_NE(GetDenormalsAreZeroMode(),
            static_cast<unsigned>(kDenormalsAreZeroOn))
      << "No DAZ enabled out of scope 1";

  {
    ScopedFloatFlushToZeroMode scoped_float_ftz_mode{
        ScopedFloatFlushToZeroFlags::kFlushToZeroOff};

    EXPECT_NE(GetFlushToZeroMode(), kFlushToZeroOn)
        << "FTZ is disabled in scope 2";

    ScopedFloatDenormalsAreZeroMode scoped_float_daz_mode{
        ScopedFloatDenormalsAreZeroFlags::kDenormalsAreZeroOn};

    EXPECT_EQ(GetDenormalsAreZeroMode(),
              static_cast<unsigned>(kDenormalsAreZeroOn))
        << "DAZ is enabled in scope 2";
  }

  EXPECT_NE(GetFlushToZeroMode(), kFlushToZeroOn)
      << "No FTZ enabled out of scope 2";
  EXPECT_NE(GetDenormalsAreZeroMode(),
            static_cast<unsigned>(kDenormalsAreZeroOn))
      << "No DAZ enabled out of scope 2";

  {
    ScopedFloatFlushToZeroMode scoped_float_ftz_mode{
        ScopedFloatFlushToZeroFlags::kFlushToZeroOff};

    EXPECT_NE(GetFlushToZeroMode(), kFlushToZeroOn)
        << "FTZ is disabled in scope 3";

    ScopedFloatDenormalsAreZeroMode scoped_float_daz_mode{
        ScopedFloatDenormalsAreZeroFlags::kDenormalsAreZeroOff};

    EXPECT_NE(GetDenormalsAreZeroMode(),
              static_cast<unsigned>(kDenormalsAreZeroOn))
        << "DAZ is disabled in scope 3";
  }

  EXPECT_NE(GetFlushToZeroMode(), kFlushToZeroOn)
      << "No FTZ enabled out of scope 3";
  EXPECT_NE(GetDenormalsAreZeroMode(),
            static_cast<unsigned>(kDenormalsAreZeroOn))
      << "No DAZ enabled out of scope 3";

  {
    ScopedFloatFlushToZeroMode scoped_float_ftz_mode{
        ScopedFloatFlushToZeroFlags::kFlushToZeroOn};

    EXPECT_EQ(GetFlushToZeroMode(), kFlushToZeroOn)
        << "FTZ is enabled in scope 3";

    ScopedFloatDenormalsAreZeroMode scoped_float_daz_mode{
        ScopedFloatDenormalsAreZeroFlags::kDenormalsAreZeroOff};

    EXPECT_NE(GetDenormalsAreZeroMode(),
              static_cast<unsigned>(kDenormalsAreZeroOn))
        << "DAZ is disabled in scope 3";
  }

  EXPECT_NE(GetFlushToZeroMode(), kFlushToZeroOn)
      << "No FTZ enabled out of scope 4";
  EXPECT_NE(GetDenormalsAreZeroMode(),
            static_cast<unsigned>(kDenormalsAreZeroOn))
      << "No DAZ enabled out of scope 4";
}

#else
#error \
    "Please define ScopedFloatFlushToZeroMode & ScopedFloatDenormalsAreZeroMode for your platform."
#endif
