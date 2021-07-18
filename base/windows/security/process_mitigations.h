// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Controls process security options.

#ifndef WB_BASE_WINDOWS_SECURITY_PROCESS_MITIGATIONS_H_
#define WB_BASE_WINDOWS_SECURITY_PROCESS_MITIGATIONS_H_

#include <system_error>
#include <tuple>

#include "base/base_api.h"
#include "base/base_macroses.h"
#include "base/windows/windows_light.h"

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
class ScopedProcessMitigationPolicies {
 public:
  WB_BASE_API ScopedProcessMitigationPolicies() noexcept;

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedProcessMitigationPolicies);

  WB_BASE_API ~ScopedProcessMitigationPolicies() noexcept;

  /**
   * @brief Gets process mitigations policies apply error code.
   * @return Error code.
   */
  [[nodiscard]] std::error_code error_code() const noexcept {
    return error_code_;
  }

 private:
  /**
   * @brief Policies apply error code.
   */
  std::error_code error_code_;

  /**
   * @brief Old policy + set new one error code.
   * @tparam TPolicy.
   */
  template <typename TPolicy>
  using old_policy_2_new_errc = std::tuple<TPolicy, std::error_code>;

  /**
   * @brief Data Execution Prevention policy.
   */
  old_policy_2_new_errc<PROCESS_MITIGATION_DEP_POLICY>
      old_dep_policy_to_new_errc_;
  /**
   * @brief Address Space Layout Randomization (ASLR) policy.
   */
  old_policy_2_new_errc<PROCESS_MITIGATION_ASLR_POLICY>
      old_aslr_policy_to_new_errc_;
  /**
   * @brief The dynamic code policy of the process.  When turned on, the process
   * cannot generate dynamic code or modify existing executable code.
   */
  old_policy_2_new_errc<PROCESS_MITIGATION_DYNAMIC_CODE_POLICY>
      old_dcp_policy_to_new_errc_;
  /**
   * @brief The process will receive a fatal error if it manipulates a handle
   * that is not valid.
   */
  old_policy_2_new_errc<PROCESS_MITIGATION_STRICT_HANDLE_CHECK_POLICY>
      old_shc_policy_to_new_errc_;
  /**
   * @brief Prevents certain built-in third party extension points from being
   * enabled, preventing legacy extension point DLLs from being loaded into the
   * process.
   */
  old_policy_2_new_errc<PROCESS_MITIGATION_EXTENSION_POINT_DISABLE_POLICY>
      old_epd_policy_to_new_errc_;
  /**
   * @brief The Control Flow Guard (CFG) policy of the process.  All DLLs that
   * are loaded must enable CFG.
   */
  old_policy_2_new_errc<PROCESS_MITIGATION_CONTROL_FLOW_GUARD_POLICY>
      old_cfg_policy_to_new_errc_;
  /**
   * @brief Disable nonsystem fonts policy.
   */
  old_policy_2_new_errc<PROCESS_MITIGATION_FONT_DISABLE_POLICY>
      old_sfd_policy_to_new_errc_;
  /**
   * @brief The policy that turns off the ability of the process to load images
   * from some locations, such a remote devices or files that have the low
   * mandatory label.
   */
  old_policy_2_new_errc<PROCESS_MITIGATION_IMAGE_LOAD_POLICY>
      old_sil_policy_to_new_errc_;
  /**
   * @brief Process mitigation policy settings for user-mode Hardware-enforced
   * Stack Protection (HSP).
   */
  old_policy_2_new_errc<PROCESS_MITIGATION_USER_SHADOW_STACK_POLICY>
      old_uss_policy_to_new_errc_;
};
}  // namespace wb::base::windows::security

#endif  // !WB_BASE_WINDOWS_SECURITY_PROCESS_MITIGATIONS_H_
