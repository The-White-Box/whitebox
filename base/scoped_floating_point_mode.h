// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Scoped floating point mode.  Useful when flush-to-zero and
// denormals-are-zero mode (SSE2) required for floating point operations.

#ifndef WB_BASE_SCOPED_FLOATING_POINT_MODE_H_
#define WB_BASE_SCOPED_FLOATING_POINT_MODE_H_

#include "build/build_config.h"

#if defined(WB_ARCH_CPU_X86_64)
#include <pmmintrin.h>
#include <xmmintrin.h>
#endif

#include "base/deps/g3log/g3log.h"
#include "base/macroses.h"
#include "build/build_config.h"

namespace wb::base {

#if defined(WB_ARCH_CPU_X86_64)
/**
 * @brief Floating point mode flags.
 */
enum class ScopedFloatingPointModeFlags : unsigned {
  /**
   * @brief No flags.  Disables flush to zero mode when float overflows.
   */
  kNone = 0x0000,
  /**
   * @brief Flush to zero when float underflows.  FTZ is an output filter:
   * Denormal inputs work normally, with no effect on compare instructions.
   * However, ordinary math instructions can't generate denormals.  Denormal
   * results are flushed to +/-0.0.  DBL_MIN/2 + DBL_MIN/1.5 should give the
   * same result as without FTZ (e.g. if those input denormals are loaded as
   * constants).
   */
  kFlushToZero = _MM_FLUSH_ZERO_ON,
  /**
   * @brief Denormals are zero.  DAZ is an input filter: When an FP math /
   * compare instruction reads its inputs, denormals are considered +/-0.0.  So
   * a compare between two denormals finds they're equal.  Arithmetic can easily
   * produce denormal results, though.  May be not supported by CPU.
   */
  kDenormalsAreZero = _MM_DENORMALS_ZERO_ON
};

/**
 * @brief | operator for ScopedFloatingPointModeFlags.
 * @param left
 * @param right
 * @return left | right.
 */
[[nodiscard]] constexpr ScopedFloatingPointModeFlags operator|(
    ScopedFloatingPointModeFlags left,
    ScopedFloatingPointModeFlags right) noexcept {
  return static_cast<ScopedFloatingPointModeFlags>(underlying_cast(left) |
                                                   underlying_cast(right));
}

/**
 * @brief Enable flush-to-zero (FTZ) [sets denormal results from floating-point
 * calculations to zero] and denormals-are-zero (DAZ) [treats denormal values
 * used as input to floating-point instructions as zero] flags and revert back
 * when out of scope.  We assume target CPU supports writing to MXCSR register
 * reserved bits (GP fault otherwise).
 *
 * In Intel® processors, the FTZ and DAZ flags in the MXCSR register are used to
 * control floating-point calculations.  Intel® Streaming SIMD Extensions
 * (Intel® SSE) and Intel® Advanced Vector Extensions (Intel® AVX) instructions,
 * including scalar and vector instructions, benefit from enabling the FTZ and
 * DAZ flags.  Floating-point computations using the Intel® SSE and Intel® AVX
 * instructions are accelerated when the FTZ and DAZ flags are enabled.  This
 * improves the application's performance.
 *
 * See Intel® 64 and IA-32 Architectures Software Developer’s Manual, Volume 1
 * 11.6.6 Guidelines for Writing to the MXCSR Register.
 *
 * See
 * https://www.intel.com/content/www/us/en/develop/documentation/cpp-compiler-developer-guide-and-reference/top/compiler-reference/floating-point-operations/understanding-floating-point-operations/setting-the-ftz-and-daz-flags.html
 */
class ScopedFloatingPointMode {
 public:
  /**
   * @brief Set floating point mode flags.
   * @param new_flags Flags.
   * @return nothing.
   */
  explicit ScopedFloatingPointMode(
      ScopedFloatingPointModeFlags new_flags) noexcept
      : previous_mxcsr_register_value_{::_mm_getcsr()} {
    ::_mm_setcsr(previous_mxcsr_register_value_ | underlying_cast(new_flags));
  }

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedFloatingPointMode);

  /**
   * @brief Restore MXCSR CPU register value to the previous one.
   */
  ~ScopedFloatingPointMode() noexcept {
    ::_mm_setcsr(previous_mxcsr_register_value_);
  }

 private:
  /**
   * @brief Previous MXCSR CPU register value.
   */
  const unsigned previous_mxcsr_register_value_;
};
#else
#error "Please define floating point mode for your platform."
#endif

}  // namespace wb::base

#endif  // !WB_BASE_SCOPED_FLOATING_POINT_MODE_H_
