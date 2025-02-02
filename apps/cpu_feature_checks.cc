// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
// CPU feature checks.

#include "cpu_feature_checks.h"

#include <algorithm>

#include "build/build_config.h"

#ifdef WB_ARCH_CPU_X86_64
#include "hal/drivers/cpu/x86_64_cpu_isa.h"
#endif

namespace wb::apps {

[[nodiscard]] std::vector<CpuFeature> QueryRequiredCpuFeatures() noexcept {
#ifdef WB_ARCH_CPU_X86_64
  using namespace wb::hal::cpus::x86_64;

  WB_GCC_BEGIN_WARNING_OVERRIDE_SCOPE()
    // pad_ is not initialized, it is ok.
    WB_GCC_DISABLE_MISSING_FIELD_INITIALIZERS_WARNING()
    std::vector<CpuFeature> cpu_features{
        CpuFeature{.name = std::string{"RDTSC"},
                   .is_supported = CpuIsa::HasRdtsc()},
        CpuFeature{.name = std::string{"RDTSCP"},
                   .is_supported = CpuIsa::HasRdtscp()},
        CpuFeature{.name = std::string{"SSE"},
                   .is_supported = CpuIsa::HasSse()},
        CpuFeature{.name = std::string{"SSE2"},
                   .is_supported = CpuIsa::HasSse2()},
        CpuFeature{.name = std::string{"SSE3"},
                   .is_supported = CpuIsa::HasSse3()},
        CpuFeature{.name = std::string{"SSSE3"},
                   .is_supported = CpuIsa::HasSsse3()},
        CpuFeature{.name = std::string{"SSE4.1"},
                   .is_supported = CpuIsa::HasSse4_1()},
        CpuFeature{.name = std::string{"SSE4.2"},
                   .is_supported = CpuIsa::HasSse4_2()},
        CpuFeature{.name = std::string{"AVX"},
                   .is_supported = CpuIsa::HasAvx()},
    };
  WB_GCC_END_WARNING_OVERRIDE_SCOPE()

  const auto is_feature_supported =
      [](const CpuFeature& cpu_feature_support) noexcept {
        return cpu_feature_support.is_supported;
      };
  if (const auto missed_feature =
          std::find_if_not(std::begin(cpu_features), std::end(cpu_features),
                           is_feature_supported);
      missed_feature != std::end(cpu_features)) [[unlikely]] {
    return cpu_features;
  }

  return {};
#else
#error "Please define CPU feature checks for your architecture."
#endif
}

[[nodiscard]] std::string QueryCpuBrand() noexcept {
#ifdef WB_ARCH_CPU_X86_64
  return wb::hal::cpus::x86_64::CpuIsa::Brand();
#else
#error "Please define CPU brand for your architecture."
#endif
}

}  // namespace wb::apps