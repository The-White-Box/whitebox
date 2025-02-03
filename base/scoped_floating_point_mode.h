// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Scoped floating point mode.  Useful when flush-to-zero and
// denormals-are-zero mode (SSE2) required for floating point operations.

#ifndef WB_BASE_SCOPED_FLOATING_POINT_MODE_H_
#define WB_BASE_SCOPED_FLOATING_POINT_MODE_H_

#include <cstddef>  // std::ptrdiff_t
#include <cstdint>  // std::uint32_t

#include "build/build_config.h"

#if defined(WB_ARCH_CPU_X86_64)
#include <pmmintrin.h>
#include <xmmintrin.h>
#endif

#include "base/deps/g3log/g3log.h"
#include "base/macroses.h"

namespace wb::base {

#if defined(WB_ARCH_CPU_X86_64)
/**
 * @brief Floating point register type on x86-64.
 */
using floating_point_register_t = std::uint32_t;
#elif defined(WB_ARCH_CPU_ARM_FAMILY) || defined(WB_ARCH_CPU_ARM_NEON)
/**
 * @brief Floating point register type on ARM.
 */
using floating_point_register_t = std::ptrdiff_t;
#else
#error Please add support for type of the floating point status register.
#endif

/**
 * @brief Gets floating point status register value.
 * @return Value.
 */
inline floating_point_register_t GetFloatingPointStatusRegister() noexcept {
#if defined(WB_ARCH_CPU_X86_64)
  const floating_point_register_t fpsr{_mm_getcsr()};
#elif defined(WB_ARCH_CPU_ARM_FAMILY) || defined(WB_ARCH_CPU_ARM_NEON)
  floating_point_register_t fpsr{0};

#if defined(WB_ARCH_CPU_ARM_FAMILY)
  asm volatile("mrs %0, fpcr" : "=r"(fpsr));
#else
  asm volatile("vmrs %0, fpscr" : "=r"(fpsr));
#endif
#else
#error Please add support for getting the floating point status register.
#endif

  return fpsr;
}

/**
 * @brief Sets floating point status register value.
 * @param fpsr New value.
 */
void SetFloatingPointStatusRegister(floating_point_register_t fpsr) noexcept {
#if defined(WB_ARCH_CPU_X86_64)
  _mm_setcsr(fpsr);
#elif defined(WB_ARCH_CPU_ARM_FAMILY) || defined(WB_ARCH_CPU_ARM_NEON)
#if defined(WB_ARCH_CPU_ARM_FAMILY)
  asm volatile("msr fpcr, %0" : : "ri"(fpsr));
#else
  asm volatile("vmsr fpscr, %0" : : "ri"(fpsr));
#endif
#else
#error Please add support for setting the floating point status register.
#endif
}

#if defined(WB_ARCH_CPU_X86_64) || defined(WB_ARCH_CPU_ARM_FAMILY) || \
    defined(WB_ARCH_CPU_ARM_NEON)

/**
 * @brief Flush to zero flags.
 */
enum class ScopedFloatFlushToZeroFlags : floating_point_register_t {
#if defined(WB_ARCH_CPU_X86_64)
  /**
   * @brief Do not flush to zero when float underflows.
   */
  kFlushToZeroOff = _MM_FLUSH_ZERO_OFF,
#else
  /**
   * @brief Do not flush to zero when float underflows.
   */
  kFlushToZeroOff = static_cast<floating_point_register_t>(0),
#endif
#if defined(WB_ARCH_CPU_X86_64)
  /**
   * @brief Flush to zero when float underflows.  FTZ is an output filter:
   * Denormal inputs work normally, with no effect on compare instructions.
   * However, ordinary math instructions can't generate denormals.  Denormal
   * results are flushed to +/-0.0.  DBL_MIN/2 + DBL_MIN/1.5 should give the
   * same result as without FTZ (e.g. if those input denormals are loaded as
   * constants).
   */
  kFlushToZeroOn = _MM_FLUSH_ZERO_ON
#else
  /**
   * @brief Flush to zero when float underflows.  FTZ is an output filter:
   * Denormal inputs work normally, with no effect on compare instructions.
   * However, ordinary math instructions can't generate denormals.  Denormal
   * results are flushed to +/-0.0.  DBL_MIN/2 + DBL_MIN/1.5 should give the
   * same result as without FTZ (e.g. if those input denormals are loaded as
   * constants).
   */
  kFlushToZeroOn = static_cast<floating_point_register_t>(1) << 24;  // FZ
#endif
};

/**
 * @brief | operator for ScopedFlushToZeroFlags.
 * @param left
 * @param right
 * @return left | right.
 */
[[nodiscard]] constexpr ScopedFloatFlushToZeroFlags operator|(
    ScopedFloatFlushToZeroFlags left,
    ScopedFloatFlushToZeroFlags right) noexcept {
  return static_cast<ScopedFloatFlushToZeroFlags>(underlying_cast(left) |
                                                  underlying_cast(right));
}

/**
 * @brief Enable flush-to-zero (FTZ) [sets denormal results from floating-point
 * calculations to zero].  We assume target CPU supports writing to MXCSR
 * register reserved bits (GP fault otherwise).
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
class ScopedFloatFlushToZeroMode {
 public:
  /**
   * @brief Set flush-to-zero mode flags.
   * @param new_flags Flags.
   * @return nothing.
   */
  explicit ScopedFloatFlushToZeroMode(
      ScopedFloatFlushToZeroFlags new_flags) noexcept
      : previous_mxcsr_register_value_{GetFloatingPointStatusRegister()} {
    SetFloatingPointStatusRegister(
        (previous_mxcsr_register_value_ &
         ~std::to_underlying(ScopedFloatFlushToZeroFlags::kFlushToZeroOn)) |
        underlying_cast(new_flags));
  }

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedFloatFlushToZeroMode);

  /**
   * @brief Restore floating point status register value to the previous one.
   */
  ~ScopedFloatFlushToZeroMode() noexcept {
    SetFloatingPointStatusRegister(previous_mxcsr_register_value_);
  }

 private:
  /**
   * @brief Previous floating point status register value.
   */
  const floating_point_register_t previous_mxcsr_register_value_;
};

/**
 * @brief Denormals are zero flags.
 */
enum class ScopedFloatDenormalsAreZeroFlags : floating_point_register_t {
#if defined(WB_ARCH_CPU_X86_64)
  /**
   * @brief Denormals are not zero.
   */
  kDenormalsAreZeroOff = _MM_FLUSH_ZERO_OFF,
#else
  /**
   * @brief Denormals are not zero.
   */
  kDenormalsAreZeroOff = static_cast<floating_point_register_t>(0),
#endif
#if defined(WB_ARCH_CPU_X86_64)
  /**
   * @brief Denormals are zero.  DAZ is an input filter: When an FP math /
   * compare instruction reads its inputs, denormals are considered +/-0.0.  So
   * a compare between two denormals finds they're equal.  Arithmetic can easily
   * produce denormal results, though.  May be not supported by CPU.
   */
  kDenormalsAreZeroOn = _MM_DENORMALS_ZERO_ON
#else
  /**
   * @brief Denormals are zero.  DAZ is an input filter: When an FP math /
   * compare instruction reads its inputs, denormals are considered +/-0.0.  So
   * a compare between two denormals finds they're equal.  Arithmetic can easily
   * produce denormal results, though.  May be not supported by CPU.
   */
  kDenormalsAreZeroOn = static_cast<floating_point_register_t>(1) << 24  // FZ
#endif
};

/**
 * @brief | operator for ScopedFlushToZeroFlags.
 * @param left
 * @param right
 * @return left | right.
 */
[[nodiscard]] constexpr ScopedFloatDenormalsAreZeroFlags operator|(
    ScopedFloatDenormalsAreZeroFlags left,
    ScopedFloatDenormalsAreZeroFlags right) noexcept {
  return static_cast<ScopedFloatDenormalsAreZeroFlags>(underlying_cast(left) |
                                                       underlying_cast(right));
}

/**
 * @brief Enable denormals-are-zero (DAZ) [treats denormal values used as input
 * to floating-point instructions as zero] flags and revert back when out of
 * scope.  We assume target CPU supports writing to MXCSR register reserved bits
 * (GP fault otherwise).
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
class ScopedFloatDenormalsAreZeroMode {
 public:
  /**
   * @brief Set denormals-are-zero mode flags.
   * @param new_flags Flags.
   * @return nothing.
   */
  explicit ScopedFloatDenormalsAreZeroMode(
      ScopedFloatDenormalsAreZeroFlags new_flags) noexcept
      : previous_mxcsr_register_value_{GetFloatingPointStatusRegister()} {
    SetFloatingPointStatusRegister(
        (previous_mxcsr_register_value_ &
         ~std::to_underlying(
             ScopedFloatDenormalsAreZeroFlags::kDenormalsAreZeroOn)) |
        underlying_cast(new_flags));
  }

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedFloatDenormalsAreZeroMode);

  /**
   * @brief Restore floating point status register value to the previous one.
   */
  ~ScopedFloatDenormalsAreZeroMode() noexcept {
    SetFloatingPointStatusRegister(previous_mxcsr_register_value_);
  }

 private:
  /**
   * @brief Previous floating point status register value.
   */
  const floating_point_register_t previous_mxcsr_register_value_;
};
#else
#error \
    "Please define ScopedFloatFlushToZeroMode & ScopedFloatDenormalsAreZeroMode for your platform."
#endif

}  // namespace wb::base

#endif  // !WB_BASE_SCOPED_FLOATING_POINT_MODE_H_
