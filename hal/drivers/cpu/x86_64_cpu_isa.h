// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// x86-64 CPU instruction set architecture.

#ifndef WB_HAL_DRIVERS_CPU_X86_64_CPU_ISA_H_
#define WB_HAL_DRIVERS_CPU_X86_64_CPU_ISA_H_

#include "build/build_config.h"

#ifdef WB_ARCH_CPU_X86_64

#include <array>
#include <bitset>
#include <cstdint>
#include <string>

#include "hal/drivers/cpu/cpu_api.h"

namespace wb::hal::cpus::x86_64 {

/**
 * @brief x86-64 CPU instruction set.  Add instructions / features detection
 * when needed.
 *
 * @remarks See "Intel® 64 and IA-32 Architectures Software Developer’s Manual
 * Volume 2 (2A, 2B, 2C & 2D): Instruction Set Reference, A-Z."
 * https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-instruction-set-reference-manual-325383.pdf
 *
 * See "AMD64 Architecture Programmer’s Manual, Volume 3: General-Purpose and
 * System Instructions" https://support.amd.com/TechDocs/24594.pdf
 */
class CpuIsa final {
 public:
  static std::string Vendor() noexcept { return Q().vendor_; }
  static std::string Brand() noexcept { return Q().brand_; }

  static bool HasSse3() noexcept { return Q().f_1_ecx_[0]; }
  static bool HasPclmulqdq() noexcept { return Q().f_1_ecx_[1]; }
  static bool HasDtes() noexcept { return Q().is_intel_ && Q().f_1_ecx_[2]; }
  static bool HasMonitor() noexcept { return Q().f_1_ecx_[3]; }
  static bool HasDsCpl() noexcept { return Q().is_intel_ && Q().f_1_ecx_[4]; }
  static bool HasVmx() noexcept { return Q().is_intel_ && Q().f_1_ecx_[5]; }
  static bool HasSmx() noexcept { return Q().is_intel_ && Q().f_1_ecx_[6]; }
  static bool HasEist() noexcept { return Q().is_intel_ && Q().f_1_ecx_[7]; }
  static bool HasTm2() noexcept { return Q().is_intel_ && Q().f_1_ecx_[8]; }
  static bool HasSsse3() noexcept { return Q().f_1_ecx_[9]; }
  static bool HasCnxtId() noexcept { return Q().is_intel_ && Q().f_1_ecx_[10]; }
  static bool HasSdbg() noexcept { return Q().is_intel_ && Q().f_1_ecx_[11]; }
  static bool HasFma() noexcept { return Q().f_1_ecx_[12]; }
  static bool HasCmpxchg16b() noexcept { return Q().f_1_ecx_[13]; }
  static bool HasXtpr() noexcept { return Q().is_intel_ && Q().f_1_ecx_[14]; }
  static bool HasPdcm() noexcept { return Q().is_intel_ && Q().f_1_ecx_[15]; }
  static bool HasPcid() noexcept { return Q().is_intel_ && Q().f_1_ecx_[17]; }
  static bool HasDca() noexcept { return Q().is_intel_ && Q().f_1_ecx_[18]; }
  static bool HasSse4_1() noexcept { return Q().f_1_ecx_[19]; }
  static bool HasSse4_2() noexcept { return Q().f_1_ecx_[20]; }
  static bool HasX2apic() noexcept { return Q().is_intel_ && Q().f_1_ecx_[21]; }
  static bool HasMovbe() noexcept { return Q().f_1_ecx_[22]; }
  static bool HasPopcnt() noexcept { return Q().f_1_ecx_[23]; }
  static bool HasTscDeadline() noexcept {
    return Q().is_intel_ && Q().f_1_ecx_[24];
  }
  static bool HasAes() noexcept { return Q().f_1_ecx_[25]; }
  static bool HasXsave() noexcept { return Q().f_1_ecx_[26]; }
  static bool HasOsXsave() noexcept { return Q().f_1_ecx_[27]; }
  static bool HasAvx() noexcept { return Q().f_1_ecx_[28]; }
  static bool HasF16c() noexcept { return Q().f_1_ecx_[29]; }
  static bool HasRdrand() noexcept { return Q().f_1_ecx_[30]; }

  static bool HasFpu() noexcept { return Q().f_1_edx_[0]; }
  static bool HasVme() noexcept { return Q().f_1_edx_[1]; }
  static bool HasDe() noexcept { return Q().f_1_edx_[2]; }
  static bool HasPse() noexcept { return Q().f_1_edx_[3]; }
  static bool HasRdtsc() noexcept { return Q().f_1_edx_[4]; }
  static bool HasMsr() noexcept { return Q().f_1_edx_[5]; }
  static bool HasPae() noexcept { return Q().f_1_edx_[6]; }
  static bool HasMce() noexcept { return Q().f_1_edx_[7]; }
  static bool HasCmpxchg8b() noexcept { return Q().f_1_edx_[8]; }
  static bool HasApic() noexcept { return Q().f_1_edx_[9]; }
  static bool HasSep() noexcept { return Q().f_1_edx_[11]; }
  static bool HasMtrr() noexcept { return Q().f_1_edx_[12]; }
  static bool HasCmov() noexcept { return Q().f_1_edx_[15]; }
  static bool HasFcmov() noexcept { return HasFpu() && Q().f_1_edx_[15]; }
  static bool HasClfsh() noexcept { return Q().f_1_edx_[19]; }
  static bool HasMmx() noexcept { return Q().f_1_edx_[23]; }
  static bool HasFxsr() noexcept { return Q().f_1_edx_[24]; }
  static bool HasSse() noexcept { return Q().f_1_edx_[25]; }
  static bool HasSse2() noexcept { return Q().f_1_edx_[26]; }

  static bool HasFsgbase() noexcept { return Q().f_7_ebx_[0]; }
  static bool HasBmi1() noexcept { return Q().f_7_ebx_[3]; }
  static bool HasHle() noexcept { return Q().is_intel_ && Q().f_7_ebx_[4]; }
  static bool HasAvx2() noexcept { return Q().f_7_ebx_[5]; }
  static bool HasBmi2() noexcept { return Q().f_7_ebx_[8]; }
  static bool HasErms() noexcept { return Q().f_7_ebx_[9]; }
  static bool HasInvpcid() noexcept { return Q().f_7_ebx_[10]; }
  static bool HasRtm() noexcept { return Q().is_intel_ && Q().f_7_ebx_[11]; }
  static bool HasAvx512f() noexcept { return Q().f_7_ebx_[16]; }
  static bool HasRdseed() noexcept { return Q().f_7_ebx_[18]; }
  static bool HasAdx() noexcept { return Q().f_7_ebx_[19]; }
  static bool HasAvx512pf() noexcept { return Q().f_7_ebx_[26]; }
  static bool HasAvx512er() noexcept { return Q().f_7_ebx_[27]; }
  static bool HasAvx512cd() noexcept { return Q().f_7_ebx_[28]; }
  static bool HasSha() noexcept { return Q().f_7_ebx_[29]; }

  static bool HasPrefetchwt1() noexcept { return Q().f_7_ecx_[0]; }

  static bool HasInvariantTsc() noexcept { return Q().f_7_edx_[8]; }

  static bool HasLahfSahf() noexcept { return Q().f_81_ecx_[0]; }
  static bool HasSvm() noexcept { return Q().is_amd_ && Q().f_81_ecx_[2]; }
  // META ExtApicSpace: extended APIC space. This bit indicates the presence of
  // extended APIC register space starting at offset 400h from the "APIC Base
  // Address Register" as specified in the BKDG.
  static bool HasExtApicSpace() noexcept {
    return Q().is_amd_ && Q().f_81_ecx_[3];
  }
  static bool HasLzcnt() noexcept { return Q().is_intel_ && Q().f_81_ecx_[5]; }
  static bool HasAbm() noexcept { return Q().is_amd_ && Q().f_81_ecx_[5]; }
  static bool HasSse4a() noexcept { return Q().is_amd_ && Q().f_81_ecx_[6]; }
  // META MisAlignSse: misaligned SSE mode.
  static bool HasMisAlignSse() noexcept {
    return Q().is_amd_ && Q().f_81_ecx_[7];
  }
  static bool Has3dNowPrefetch() noexcept {
    return Q().is_amd_ && Q().f_81_ecx_[8];
  }
  // META IBS: instruction based sampling.
  static bool HasIbs() noexcept { return Q().is_amd_ && Q().f_81_ecx_[10]; }
  static bool HasXop() noexcept { return Q().is_amd_ && Q().f_81_ecx_[11]; }
  // META WDT: watchdog timer support.
  static bool HasWdt() noexcept { return Q().is_amd_ && Q().f_81_ecx_[13]; }
  // META LWP: lightweight profiling support.
  static bool HasLwp() noexcept { return Q().is_amd_ && Q().f_81_ecx_[15]; }
  static bool HasFma4() noexcept { return Q().is_amd_ && Q().f_81_ecx_[16]; }
  static bool HasTbm() noexcept { return Q().is_amd_ && Q().f_81_ecx_[21]; }

  static bool HasSyscall() noexcept { return Q().f_81_edx_[11]; }
  // META NX: has no-execute page protection.
  static bool HasNx() noexcept { return Q().is_amd_ && Q().f_81_edx_[20]; }
  static bool HasMmxExt() noexcept { return Q().is_amd_ && Q().f_81_edx_[22]; }
  // FFXSR: FXSAVE and FXRSTOR instruction optimizations.
  static bool HasFfxsr() noexcept { return Q().is_amd_ && Q().f_81_edx_[25]; }
  static bool HasRdtscp() noexcept { return Q().f_81_edx_[27]; }

  // META LM: Long Mode, or Intel 64.
  static bool HasLM() noexcept { return Q().f_81_edx_[29]; }
  static bool Has3dNowExt() noexcept {
    return Q().is_amd_ && Q().f_81_edx_[30];
  }
  static bool Has3dNow() noexcept { return Q().is_amd_ && Q().f_81_edx_[31]; }

 private:
  /**
   * @brief CPU instruction set query.
   */
  struct CpuQuery {
    /**
     * @brief Perform query of CPU instruction set.
     * @return nothing.
     */
    WB_HAL_CPU_DRIVER_API CpuQuery() noexcept;

    char vendor_[0x10], brand_[0x40];
    bool is_intel_, is_amd_;

#ifdef WB_COMPILER_MSVC
    WB_ATTRIBUTE_UNUSED_FIELD std::array<std::byte, 2> pad_;
#elif defined(WB_COMPILER_CLANG) || defined(WB_COMPILER_GCC)
    WB_ATTRIBUTE_UNUSED_FIELD std::array<std::byte, 6> pad_;
#endif

    std::bitset<32> f_1_ecx_;  //-V112
    std::bitset<32> f_1_edx_;
    std::bitset<32> f_7_ebx_;
    std::bitset<32> f_7_ecx_;
    std::bitset<32> f_7_edx_;
    std::bitset<32> f_81_ecx_;
    std::bitset<32> f_81_edx_;
  };

  /**
   * @brief Performs CPU instruction set query once.
   * @return Resolved CPU instruction set query.
   */
  [[nodiscard]] static const CpuQuery &Q() noexcept {
    static const CpuQuery q;
    return q;
  }
};

}  // namespace wb::hal::cpus::x86_64

#endif  // WB_ARCH_CPU_X86_64

#endif  // !WB_HAL_DRIVERS_CPU_X86_64_CPU_ISA_H_
