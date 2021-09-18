// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Controls process security options.

#ifndef WB_BASE_WIN_SECURITY_PROCESS_MITIGATIONS_H_
#define WB_BASE_WIN_SECURITY_PROCESS_MITIGATIONS_H_

#include <system_error>

#include "base/base_api.h"
#include "base/base_macroses.h"
#include "base/std2/system_error_ext.h"
#include "build/compiler_config.h"

namespace wb::base::windows::security {
/**
 * @brief Enables secure set of directories to search when the calling process
 * loads a DLL.  This search path is used when LoadLibraryEx is called with no
 * LOAD_LIBRARY_SEARCH flags.
 * @return Error code.
 */
[[nodiscard]] WB_BASE_API std::error_code
EnableDefaultSecureDllSearch() noexcept;

/**
 * @brief Enables process mitigation policies in scope.
 */
class WB_BASE_API ScopedProcessMitigationPolicies {
 public:
  /**
   * @brief Enables process mitigation policies in scope.
   * @return ScopedProcessMitigationPolicies.
   */
  static std2::result<ScopedProcessMitigationPolicies> New() noexcept;

  ScopedProcessMitigationPolicies(ScopedProcessMitigationPolicies&& p) noexcept;
  ScopedProcessMitigationPolicies& operator=(
      ScopedProcessMitigationPolicies&&) = delete;

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(ScopedProcessMitigationPolicies);

  ~ScopedProcessMitigationPolicies() noexcept;

 private:
  class ScopedProcessMitigationPoliciesImpl;

  WB_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()
    // Private member is not accessible to the DLL's client, including inline
    // functions.
    WB_MSVC_DISABLE_WARNING(4251)
    /**
     * @brief Actual implementation.
     */
    wb::base::un<ScopedProcessMitigationPoliciesImpl> impl_;
  WB_MSVC_END_WARNING_OVERRIDE_SCOPE()

  /**
   * @brief Creates ScopedProcessMitigationPolicies.
   * @return nothing.
   */
  ScopedProcessMitigationPolicies() noexcept;

  /**
   * @brief Gets process mitigations policies apply error code.
   * @return Error code.
   */
  [[nodiscard]] std::error_code error_code() const noexcept;
};
}  // namespace wb::base::windows::security

#endif  // !WB_BASE_WIN_SECURITY_PROCESS_MITIGATIONS_H_
