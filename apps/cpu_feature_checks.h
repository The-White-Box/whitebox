// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// CPU feature checks.

#ifndef WB_APPS_CPU_FEATURE_CHECKS_H_
#define WB_APPS_CPU_FEATURE_CHECKS_H_

#include <array>
#include <string>
#include <vector>

#include "build/compiler_config.h"

namespace wb::apps {

/**
 * @brief CPU feature support state.
 */
struct CpuFeature {
  /**
   * @brief CPU feature name.
   */
  std::string name;
  /**
   * @brief Is CPU feature supported.
   */
  bool is_supported;

  WB_ATTRIBUTE_UNUSED_FIELD
  std::array<std::byte, sizeof(char *) - sizeof(is_supported)> pad_;
};

/**
 * @brief Get required CPU features.
 * @return Empty vector in case all required CPU features are supported.  All
 * required CPU features with their support state otherwise.
 */
[[nodiscard]] std::vector<CpuFeature> QueryRequiredCpuFeatures() noexcept;

/**
 * @brief Get CPU brand.
 * @return CPU brand.
 */
[[nodiscard]] std::string QueryCpuBrand() noexcept;

}  // namespace wb::apps

#endif  // !WB_APPS_CPU_FEATURE_CHECKS_H_
