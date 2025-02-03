// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// x86-64 CPU instruction set architecture.

#include "x86_64_cpu_isa.h"

#include "build/compiler_config.h"  // WB_COMPILER_*

#if defined(WB_COMPILER_MSVC)
#include <intrin.h>
#endif

#include <vector>

#include "base/std2/cstring_ext.h"
#include "base/std2/string_view_ext.h"
#include "hal/drivers/cpu/cpu_api.h"

#ifdef WB_ARCH_CPU_X86_64

namespace {

/**
 * @brief CPUID instruction wrapper.
 * @param function_id A code that specifies the information to retrieve, passed
 * in EAX.
 * @return An array of four integers that contains the information returned in
 * EAX, EBX, ECX, and EDX about supported features of the CPU.
 */
[[nodiscard]] inline std::array<std::int32_t, 4> cpuid(
    std::int32_t function_id) noexcept {
  std::array<std::int32_t, 4> info = {};  //-V112

#if defined(WB_COMPILER_CLANG) || defined(WB_COMPILER_GCC)
#ifdef WB_ARCH_CPU_X86_64
  asm volatile(
      "movq\t%%rbx, %%rsi\n\t"
      "cpuid\n\t"
      "xchgq\t%%rbx, %%rsi\n\t"
      : "=a"(info[0]), "=S"(info[1]), "=c"(info[2]), "=d"(info[3])
      : "a"(function_id));
  return info;
#else
#error "Please add cpuid support for your arhitecture."
  return info;
#endif  // WB_ARCH_CPU_X86_64
#elif defined(WB_COMPILER_MSVC)
  __cpuid(info.data(), function_id);
  return info;
#endif
}

/**
 * @brief CPUIDEX intrinsic wrapper.
 * @param function_id A code that specifies the information to retrieve, passed
 * in EAX.
 * @param function_id An additional code that specifies information to retrieve,
 * passed in ECX.
 * @return An array of four integers that contains the information returned in
 * EAX, EBX, ECX, and EDX about supported features of the CPU.
 */
[[nodiscard]] inline std::array<std::int32_t, 4> cpuidex(
    std::int32_t function_id, std::int32_t subfunction_id) noexcept {
  std::array<std::int32_t, 4> info = {};

#if defined(WB_COMPILER_CLANG) || defined(WB_COMPILER_GCC)
#ifdef WB_ARCH_CPU_X86_64
  // The __cpuidex intrinsic sets the value of the ECX register to
  // subfunction_id before it generates the cpuid instruction.
  asm volatile(
      "movq\t%%rbx, %%rsi\n\t"
      "cpuid\n\t"
      "xchgq\t%%rbx, %%rsi\n\t"
      : "=a"(info[0]), "=S"(info[1]), "=c"(info[2]), "=d"(info[3])
      : "a"(function_id), "c"(subfunction_id));
  return info;
#else
#error "Please add cpuidex support for your arhitecture."
  return info;
#endif  // WB_ARCH_CPU_X86_64
#elif defined(WB_COMPILER_MSVC)
  __cpuidex(info.data(), function_id, subfunction_id);
  return info;
#endif
}

/**
 * @brief Get CPU vendor.
 * @param in CPU registers to extract vendor.
 * @param vendor CPU vendor.
 * @return CPU vendor.
 */
std::string GetVendor(const std::array<std::int32_t, 4> &in) noexcept {
  char vendor[0x10];

  static_assert(sizeof(vendor) >= sizeof(std::int32_t) * 3,
                "CPU vendor string has at least 12 bytes.");

  std::memcpy(vendor, &in[1], sizeof(in[1]));
  std::memcpy(vendor + sizeof(in[1]), &in[3], sizeof(in[3]));
  std::memcpy(vendor + sizeof(in[1]) + sizeof(in[3]), &in[2],  //-V119
              sizeof(in[2]));

  return vendor;
}

/**
 * @brief Get CPU brand.
 * @param in Input data.
 * @param brand CPU brand.
 * @return CPU brand.
 */
std::string GetBrand(
    const std::vector<std::array<std::int32_t, 4>> &in) noexcept {
  char brand_temp[0x40]{'\0'}, brand_out[0x40]{'\0'};

  static_assert(sizeof(brand_temp) == sizeof(brand_out),
                "CPU brand string has at least 48 bytes.");
  static_assert(sizeof(brand_temp) >= sizeof(in[2]) * 3,
                "CPU brand string has at least 48 bytes.");

  wb::base::std2::BitwiseCopy(brand_temp, in[2]);
  std::memcpy(brand_temp + sizeof(in[2]), in[3].data(), sizeof(in[3]));
  std::memcpy(brand_temp + sizeof(in[2]) + sizeof(in[3]), in[4].data(),  //-V119
              sizeof(in[4]));

  return wb::base::std2::TrimSpaces(brand_temp, brand_out) ? brand_out
                                                           : brand_temp;
}

}  // namespace

namespace wb::hal::cpus::x86_64 {

WB_HAL_CPU_DRIVER_API CpuIsa::CpuQuery::CpuQuery() noexcept
    : vendor_{'\0'},
      brand_{'\0'},
      is_intel_{false},
      is_amd_{false},
      f_1_ecx_{0},
      f_1_edx_{0},
      f_7_ebx_{0},
      f_7_ecx_{0},
      f_7_edx_{0},
      f_81_ecx_{0},
      f_81_edx_{0} {
  // Calling cpuid with 0x0 as the function_id argument gets the number
  // of the highest valid function ID.
  std::array<std::int32_t, 4> info{cpuid(0)};
  const int32_t func_ids_count{info[0]};

  std::vector<std::array<std::int32_t, 4>> data;
  data.reserve(static_cast<std::size_t>(std::max(func_ids_count, 0)) +  //-V201
               1U);

  for (std::int32_t i{0}; i <= func_ids_count; ++i) {
    data.emplace_back(cpuidex(i, 0));
  }

  if (func_ids_count >= 0) {
    // Capture vendor string.
    vendor_ = GetVendor(data[0]);

    if (vendor_ == "GenuineIntel") {
      is_intel_ = true;
    } else if (vendor_ == "AuthenticAMD") {
      is_amd_ = true;
    }
  }

  // Load bitset with flags for function 0x00000001.
  if (func_ids_count >= 1) {
    f_1_ecx_ = static_cast<std::uint32_t>(data[1][2]);
    f_1_edx_ = static_cast<std::uint32_t>(data[1][3]);
  }

  // Load bitset with flags for function 0x00000007.
  if (func_ids_count >= 7) {
    f_7_ebx_ = static_cast<std::uint32_t>(data[7][1]);
    f_7_ecx_ = static_cast<std::uint32_t>(data[7][2]);
    f_7_edx_ = static_cast<std::uint32_t>(data[7][3]);
  }

  // Calling cpuid with 0x80000000 as the function_id argument gets the
  // number of the highest valid extended ID.
  info = cpuid(static_cast<std::int32_t>(0x80000000U));  //-V112
  const int32_t ext_func_ids_count{info[0]};

  std::vector<std::array<std::int32_t, 4>> ext_data;
  ext_data.reserve(static_cast<std::size_t>(  //-V201
                       std::max(ext_func_ids_count - static_cast<std::int32_t>(
                                                         0x80000000),  //-V112
                                0)) +
                   1U);

  for (std::int32_t i{static_cast<std::int32_t>(0x80000000)};  //-V112
       i <= ext_func_ids_count; ++i) {
    ext_data.emplace_back(cpuidex(i, 0));
  }

  // Load bitset with flags for function 0x80000001.
  if (static_cast<unsigned>(ext_func_ids_count) >= 0x80000001U) {
    f_81_ecx_ = static_cast<std::uint32_t>(ext_data[1][2]);
    f_81_edx_ = static_cast<std::uint32_t>(ext_data[1][3]);
  }

  // Interpret cpu brand string if reported.
  if (static_cast<unsigned>(ext_func_ids_count) >= 0x80000004U) {
    brand_ = std::move(GetBrand(ext_data));
  }
}

}  // namespace wb::hal::cpus::x86_64

#endif  // WB_ARCH_CPU_X86_64