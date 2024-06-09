// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.

#include "process_mitigations.h"

#include <tuple>

#include "base/deps/g3log/g3log.h"
#include "base/std2/cstring_ext.h"
#include "base/win/system_error_ext.h"
#include "base/win/windows_light.h"
#include "base/win/windows_version.h"

namespace wb::base::win::security {

/**
 * @brief Enables secure set of directories to search when the calling process
 * loads a DLL.  This search path is used when LoadLibraryEx is called with no
 * LOAD_LIBRARY_SEARCH flags.
 * @return Error code.
 */
[[nodiscard]] WB_BASE_API std::error_code
EnableDefaultSecureDllSearch() noexcept {
  // Specifies a default set of directories to search when the calling process
  // loads a DLL.  This search path is used when LoadLibraryEx is called with no
  // LOAD_LIBRARY_SEARCH flags.
  //
  // See
  // https://docs.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-setdefaultdlldirectories
  return get_error(::SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_SYSTEM32 |
                                              LOAD_LIBRARY_SEARCH_USER_DIRS));
}

/**
 * @brief Computes mitigation policy flag based on policy type.
 * @tparam TPolicy Policy to get mitigation flag for.
 */
template <typename TPolicy>
constexpr PROCESS_MITIGATION_POLICY mitigation_policy_flag =
    std::is_same_v<TPolicy, PROCESS_MITIGATION_DEP_POLICY> ? ProcessDEPPolicy
    : std::is_same_v<TPolicy, PROCESS_MITIGATION_ASLR_POLICY>
        ? ProcessASLRPolicy
    : std::is_same_v<TPolicy, PROCESS_MITIGATION_DYNAMIC_CODE_POLICY>
        ? ProcessDynamicCodePolicy
    : std::is_same_v<TPolicy, PROCESS_MITIGATION_STRICT_HANDLE_CHECK_POLICY>
        ? ProcessStrictHandleCheckPolicy
    : std::is_same_v<TPolicy, PROCESS_MITIGATION_SYSTEM_CALL_DISABLE_POLICY>
        ? ProcessSystemCallDisablePolicy
    : std::is_same_v<TPolicy, PROCESS_MITIGATION_EXTENSION_POINT_DISABLE_POLICY>
        ? ProcessExtensionPointDisablePolicy
    : std::is_same_v<TPolicy, PROCESS_MITIGATION_CONTROL_FLOW_GUARD_POLICY>
        ? ProcessControlFlowGuardPolicy
    : std::is_same_v<TPolicy, PROCESS_MITIGATION_FONT_DISABLE_POLICY>
        ? ProcessFontDisablePolicy
    : std::is_same_v<TPolicy, PROCESS_MITIGATION_IMAGE_LOAD_POLICY>
        ? ProcessImageLoadPolicy
    : std::is_same_v<TPolicy, PROCESS_MITIGATION_REDIRECTION_TRUST_POLICY>
        ? ProcessRedirectionTrustPolicy
    : std::is_same_v<TPolicy, PROCESS_MITIGATION_USER_SHADOW_STACK_POLICY>
        ? ProcessUserShadowStackPolicy
    : std::is_same_v<TPolicy, PROCESS_MITIGATION_USER_POINTER_AUTH_POLICY>
        ? ProcessUserPointerAuthPolicy
    : std::is_same_v<TPolicy, PROCESS_MITIGATION_SEHOP_POLICY>
        ? ProcessSEHOPPolicy
        : MaxProcessMitigationPolicy;

/**
 * @brief Mitigation policy concept.
 * @tparam TPolicy Mitigation policy.
 * @tparam R Return type.
 */
template <typename TPolicy>
concept mitigation_policy =
    mitigation_policy_flag<TPolicy> != MaxProcessMitigationPolicy;

/**
 * @brief Get mitigation policy setting from OS.
 * @tparam TPolicy Policy to get.
 * @param process Process handle.
 * @param policy Policy to fill with OS data.
 * @return Error code.
 */
template <mitigation_policy TPolicy>
[[nodiscard]] std::error_code GetProcessMitigationPolicy(
    HANDLE process, TPolicy& policy) noexcept {
  std2::BitwiseMemset(policy, 0);

  constexpr PROCESS_MITIGATION_POLICY policy_flag{
      mitigation_policy_flag<TPolicy>};
  return get_error(::GetProcessMitigationPolicy(process, policy_flag, &policy,
                                                sizeof(policy)));
}

/**
 * @brief Set mitigation policy setting in OS.
 * @tparam TPolicy Policy to set.
 * @param policy Policy data to set in OS.
 * @return Error code.
 */
template <mitigation_policy TPolicy>
[[nodiscard]] std::error_code SetProcessMitigationPolicy(
    TPolicy& policy) noexcept {
  constexpr PROCESS_MITIGATION_POLICY policy_flag{
      mitigation_policy_flag<TPolicy>};
  const std::error_code rc{get_error(
      ::SetProcessMitigationPolicy(policy_flag, &policy, sizeof(policy)))};

  // Access denied is ok result.
  return !rc || rc.value() == ERROR_ACCESS_DENIED ? std2::ok_code : rc;
}

/**
 * @brief Impementation for enabler of process mitigation policies in scope.
 */
class ScopedProcessMitigationPolicies::ScopedProcessMitigationPoliciesImpl
    final {
 public:
  ScopedProcessMitigationPoliciesImpl() noexcept;

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedProcessMitigationPoliciesImpl);

  ~ScopedProcessMitigationPoliciesImpl() noexcept;

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
  template <mitigation_policy TPolicy>
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
   * @brief Process mitigation policy settings for Redirection Guard.
   */
  old_policy_2_new_errc<PROCESS_MITIGATION_REDIRECTION_TRUST_POLICY>
      old_srt_policy_to_new_errc_;
  /**
   * @brief Process mitigation policy settings for user-mode Hardware-enforced
   * Stack Protection (HSP).  Windows 10, version 2004 and above.
   */
  old_policy_2_new_errc<PROCESS_MITIGATION_USER_SHADOW_STACK_POLICY>
      old_uss_policy_to_new_errc_;
  /**
   * @brief Process mitigation policy settings for Redirection Guard.  Windows
   * 11, version 22H2.
   */
  old_policy_2_new_errc<PROCESS_MITIGATION_USER_POINTER_AUTH_POLICY>
      old_upa_policy_to_new_errc_;
  /**
   * @brief Process mitigation policy settings for user-mode Hardware-enforced
   * Stack Protection (HSP).  Windows 11, version 22H2.
   */
  old_policy_2_new_errc<PROCESS_MITIGATION_SEHOP_POLICY>
      old_sop_policy_to_new_errc_;
};

ScopedProcessMitigationPolicies::ScopedProcessMitigationPoliciesImpl::
    ScopedProcessMitigationPoliciesImpl() noexcept
    : error_code_{},
      old_dep_policy_to_new_errc_{},
      old_aslr_policy_to_new_errc_{},
      old_dcp_policy_to_new_errc_{},
      old_shc_policy_to_new_errc_{},
      old_epd_policy_to_new_errc_{},
      old_cfg_policy_to_new_errc_{},
      old_sfd_policy_to_new_errc_{},
      old_sil_policy_to_new_errc_{},
      old_srt_policy_to_new_errc_{},
      old_uss_policy_to_new_errc_{},
      old_upa_policy_to_new_errc_{},
      old_sop_policy_to_new_errc_{} {
  // NOLINTNEXTLINE(misc-misplaced-const)
  const HANDLE current_process{::GetCurrentProcess()};

  std::error_code rc = GetProcessMitigationPolicy(
      current_process,
      std::get<PROCESS_MITIGATION_DEP_POLICY>(old_dep_policy_to_new_errc_));
  if (!rc) {
    const auto& old_policy =
        std::get<PROCESS_MITIGATION_DEP_POLICY>(old_dep_policy_to_new_errc_);

    if (!old_policy.Permanent &&
        (!old_policy.Enable || !old_policy.DisableAtlThunkEmulation)) {
      PROCESS_MITIGATION_DEP_POLICY new_policy{.Enable = 1,
                                               .DisableAtlThunkEmulation = 1};

      if ((std::get<std::error_code>(old_dep_policy_to_new_errc_) =
              SetProcessMitigationPolicy(new_policy))) {
        error_code_ = std::get<std::error_code>(old_dep_policy_to_new_errc_);

        G3PLOG_E(WARNING, error_code_)
            << "Can't enable 'Data Execution Prevention' process mitigation "
               "policy, app will run with no/partial data execution "
               "prevention.";
      }
    }
  } else {
    std::get<std::error_code>(old_dep_policy_to_new_errc_) = rc;

    G3PLOG_E(WARNING, rc)
        << "Can't enable 'Data Execution Prevention' process mitigation "
           "policy, app will run with no/partial data execution prevention.";
  }

  rc = GetProcessMitigationPolicy(
      current_process,
      std::get<PROCESS_MITIGATION_ASLR_POLICY>(old_aslr_policy_to_new_errc_));
  if (!rc) {
    const auto& old_policy =
        std::get<PROCESS_MITIGATION_ASLR_POLICY>(old_aslr_policy_to_new_errc_);

    if (!old_policy.EnableBottomUpRandomization ||
        !old_policy.EnableForceRelocateImages ||
        !old_policy.EnableHighEntropy || !old_policy.DisallowStrippedImages) {
      PROCESS_MITIGATION_ASLR_POLICY new_policy{
          .EnableBottomUpRandomization = 1,
          .EnableForceRelocateImages = 1,
          .EnableHighEntropy = 1,
          .DisallowStrippedImages = 1};

      if ((std::get<std::error_code>(old_aslr_policy_to_new_errc_) =
              SetProcessMitigationPolicy(new_policy))) {
        error_code_ = std::get<std::error_code>(old_aslr_policy_to_new_errc_);

        G3PLOG_E(WARNING, error_code_)
            << "Can't enable 'Address Space Layout Randomization' process "
               "mitigation policy, app will run with no/partial address space "
               "layout randomization.";
      }
    }
  } else {
    std::get<std::error_code>(old_aslr_policy_to_new_errc_) = rc;

    G3PLOG_E(WARNING, rc)
        << "Can't enable 'Address Space Layout Randomization' process "
           "mitigation policy, app will run with no/partial address space "
           "layout randomization.";
  }

  rc = GetProcessMitigationPolicy(
      current_process, std::get<PROCESS_MITIGATION_DYNAMIC_CODE_POLICY>(
                           old_dcp_policy_to_new_errc_));
  if (!rc) {
    const auto& old_policy = std::get<PROCESS_MITIGATION_DYNAMIC_CODE_POLICY>(
        old_dcp_policy_to_new_errc_);

    if (!old_policy.ProhibitDynamicCode || old_policy.AllowThreadOptOut ||
        old_policy.AllowRemoteDowngrade) {
      PROCESS_MITIGATION_DYNAMIC_CODE_POLICY new_policy{
          .ProhibitDynamicCode = 1,
          .AllowThreadOptOut = 0,
          .AllowRemoteDowngrade = 0};

      if ((std::get<std::error_code>(old_dcp_policy_to_new_errc_) =
              SetProcessMitigationPolicy(new_policy))) {
        error_code_ = std::get<std::error_code>(old_dcp_policy_to_new_errc_);

        G3PLOG_E(WARNING, error_code_)
            << "Can't enable 'Dynamic Code Prohibition' process mitigation "
               "policy, app will run with no/partial dynamic code prohibition.";
      }
    }
  } else {
    std::get<std::error_code>(old_dcp_policy_to_new_errc_) = rc;

    G3PLOG_E(WARNING, error_code_)
        << "Can't enable 'Dynamic Code Prohibition' process mitigation policy, "
           "app will run with no/partial dynamic code prohibition.";
  }

  rc = GetProcessMitigationPolicy(
      current_process, std::get<PROCESS_MITIGATION_STRICT_HANDLE_CHECK_POLICY>(
                           old_shc_policy_to_new_errc_));
  if (!rc) {
    const auto& old_policy =
        std::get<PROCESS_MITIGATION_STRICT_HANDLE_CHECK_POLICY>(
            old_shc_policy_to_new_errc_);

    if (!old_policy.RaiseExceptionOnInvalidHandleReference ||
        !old_policy.HandleExceptionsPermanentlyEnabled) {
      PROCESS_MITIGATION_STRICT_HANDLE_CHECK_POLICY new_policy{
          .RaiseExceptionOnInvalidHandleReference = 1,
          .HandleExceptionsPermanentlyEnabled = 1};

      if ((std::get<std::error_code>(old_shc_policy_to_new_errc_) =
              SetProcessMitigationPolicy(new_policy))) {
        error_code_ = std::get<std::error_code>(old_shc_policy_to_new_errc_);

        G3PLOG_E(WARNING, error_code_)
            << "Can't enable 'Strict Handle Check' process mitigation policy, "
               "app will run with no/partial strict handle checking.";
      }
    }
  } else {
    std::get<std::error_code>(old_shc_policy_to_new_errc_) = rc;

    G3PLOG_E(WARNING, rc)
        << "Can't enable 'Strict Handle Check' process mitigation policy, "
           "app will run with no/partial strict handle checking.";
  }

  rc = GetProcessMitigationPolicy(
      current_process,
      std::get<PROCESS_MITIGATION_EXTENSION_POINT_DISABLE_POLICY>(
          old_epd_policy_to_new_errc_));
  if (!rc) {
    const auto& old_policy =
        std::get<PROCESS_MITIGATION_EXTENSION_POINT_DISABLE_POLICY>(
            old_epd_policy_to_new_errc_);

    if (!old_policy.DisableExtensionPoints) {
      PROCESS_MITIGATION_EXTENSION_POINT_DISABLE_POLICY new_policy{
          .DisableExtensionPoints = 1};

      if ((std::get<std::error_code>(old_epd_policy_to_new_errc_) =
              SetProcessMitigationPolicy(new_policy))) {
        error_code_ = std::get<std::error_code>(old_epd_policy_to_new_errc_);

        G3PLOG_E(WARNING, error_code_)
            << "Can't enable 'Extension Point Disablement' process mitigation "
               "policy, app will run with not disabled extension points.";
      }
    }
  } else {
    std::get<std::error_code>(old_epd_policy_to_new_errc_) = rc;

    G3PLOG_E(WARNING, rc)
        << "Can't enable 'Extension Point Disablement' process mitigation "
           "policy, app will run with not disabled extension points.";
  }

  rc = GetProcessMitigationPolicy(
      current_process, std::get<PROCESS_MITIGATION_CONTROL_FLOW_GUARD_POLICY>(
                           old_cfg_policy_to_new_errc_));
  if (!rc) {
    const auto& old_policy =
        std::get<PROCESS_MITIGATION_CONTROL_FLOW_GUARD_POLICY>(
            old_cfg_policy_to_new_errc_);
    // Only known option to be altered via API for now.
    if (!old_policy.StrictMode) {
      PROCESS_MITIGATION_CONTROL_FLOW_GUARD_POLICY new_policy{.StrictMode =
                                                                  TRUE};

      if ((std::get<std::error_code>(old_cfg_policy_to_new_errc_) =
              SetProcessMitigationPolicy(new_policy))) {
        error_code_ = std::get<std::error_code>(old_cfg_policy_to_new_errc_);

        G3PLOG_E(WARNING, error_code_)
            << "Can't enable 'DLLs must enable CFG' process mitigation "
               "policy, app will run with DLLs which not enable CFG.";
      }
    }
  } else {
    std::get<std::error_code>(old_cfg_policy_to_new_errc_) = rc;

    G3PLOG_E(WARNING, rc)
        << "Can't enable 'DLLs must enable CFG' process mitigation "
           "policy, app will run with DLLs which not enable CFG.";
  }

  rc = GetProcessMitigationPolicy(
      current_process, std::get<PROCESS_MITIGATION_FONT_DISABLE_POLICY>(
                           old_sfd_policy_to_new_errc_));
  if (!rc) {
    const auto& old_policy = std::get<PROCESS_MITIGATION_FONT_DISABLE_POLICY>(
        old_sfd_policy_to_new_errc_);
    if (!old_policy.DisableNonSystemFonts) {
      PROCESS_MITIGATION_FONT_DISABLE_POLICY new_policy{.DisableNonSystemFonts =
                                                            TRUE};

      if ((std::get<std::error_code>(old_sfd_policy_to_new_errc_) =
              SetProcessMitigationPolicy(new_policy))) {
        error_code_ = std::get<std::error_code>(old_sfd_policy_to_new_errc_);

        G3PLOG_E(WARNING, error_code_)
            << "Can't enable 'Only system fonts' process mitigation "
               "policy, app will run with custom fonts.";
      }
    }
  } else {
    std::get<std::error_code>(old_sfd_policy_to_new_errc_) = rc;

    G3PLOG_E(WARNING, rc)
        << "Can't enable 'Only system fonts' process mitigation "
           "policy, app will run with custom fonts.";
  }

  rc = GetProcessMitigationPolicy(
      current_process, std::get<PROCESS_MITIGATION_IMAGE_LOAD_POLICY>(
                           old_sil_policy_to_new_errc_));
  if (!rc) {
    const auto& old_policy = std::get<PROCESS_MITIGATION_IMAGE_LOAD_POLICY>(
        old_sil_policy_to_new_errc_);
    if (!old_policy.NoRemoteImages || !old_policy.NoLowMandatoryLabelImages ||
        !old_policy.PreferSystem32Images) {
      PROCESS_MITIGATION_IMAGE_LOAD_POLICY new_policy{
          .NoRemoteImages = TRUE,
          .NoLowMandatoryLabelImages = TRUE,
          .PreferSystem32Images = TRUE};

      if ((std::get<std::error_code>(old_sil_policy_to_new_errc_) =
              SetProcessMitigationPolicy(new_policy))) {
        error_code_ = std::get<std::error_code>(old_sil_policy_to_new_errc_);

        G3PLOG_E(WARNING, error_code_)
            << "Can't enable 'Strict Image Load' process mitigation "
               "policy, app will load images from unsafe locations.";
      }
    }
  } else {
    std::get<std::error_code>(old_sil_policy_to_new_errc_) = rc;

    G3PLOG_E(WARNING, rc)
        << "Can't enable 'Strict Image Load' process mitigation "
           "policy, app will load images from unsafe locations.";
  }

  rc = GetProcessMitigationPolicy(
      current_process, std::get<PROCESS_MITIGATION_REDIRECTION_TRUST_POLICY>(
                           old_srt_policy_to_new_errc_));
  if (!rc) {
    const auto& old_policy =
        std::get<PROCESS_MITIGATION_REDIRECTION_TRUST_POLICY>(
            old_srt_policy_to_new_errc_);
    if (!old_policy.EnforceRedirectionTrust) {
      PROCESS_MITIGATION_REDIRECTION_TRUST_POLICY new_policy{
          .EnforceRedirectionTrust = TRUE};

      if ((std::get<std::error_code>(old_srt_policy_to_new_errc_) =
              SetProcessMitigationPolicy(new_policy))) {
        error_code_ = std::get<std::error_code>(old_srt_policy_to_new_errc_);

        G3PLOG_E(WARNING, error_code_)
            << "Can't enable 'Redirection Guard' process mitigation policy, "
               "app will follow filesystem junctions created by non-admin "
               "users and doesn't log such attempts.";
      }
    }
  } else {
    std::get<std::error_code>(old_srt_policy_to_new_errc_) = rc;

    G3PLOG_E(WARNING, rc)
        << "Can't enable 'Redirection Guard' process mitigation policy, app "
           "will follow filesystem junctions created by non-admin users "
           "and doesn't log such attempts.";
  }

  if (win::GetVersion() < win::Version::WIN10_20H1) [[unlikely]] {
    // Policies below require Windows 10, version 2004+ (Build 19041)
    std::get<std::error_code>(old_uss_policy_to_new_errc_) =
        std2::system_last_error_code(ERROR_NOT_SUPPORTED);
    std::get<std::error_code>(old_upa_policy_to_new_errc_) =
        std2::system_last_error_code(ERROR_NOT_SUPPORTED);
    std::get<std::error_code>(old_sop_policy_to_new_errc_) =
        std2::system_last_error_code(ERROR_NOT_SUPPORTED);
    return;
  }

  rc = GetProcessMitigationPolicy(
      current_process, std::get<PROCESS_MITIGATION_USER_SHADOW_STACK_POLICY>(
                           old_uss_policy_to_new_errc_));
  if (!rc) {
    const auto& old_policy =
        std::get<PROCESS_MITIGATION_USER_SHADOW_STACK_POLICY>(
            old_uss_policy_to_new_errc_);
    if (!old_policy.EnableUserShadowStack || !old_policy.BlockNonCetBinaries) {
      PROCESS_MITIGATION_USER_SHADOW_STACK_POLICY new_policy{
          .EnableUserShadowStack = TRUE, .BlockNonCetBinaries = TRUE};

      if ((std::get<std::error_code>(old_uss_policy_to_new_errc_) =
              SetProcessMitigationPolicy(new_policy))) {
        error_code_ = std::get<std::error_code>(old_uss_policy_to_new_errc_);

        G3PLOG_E(WARNING, error_code_)
            << "Can't enable 'User-mode Hardware-enforced Stack Protection' "
               "process mitigation policy, app will not apply shadow stacks "
               "protection mechanism.";
      }
    }
  } else {
    std::get<std::error_code>(old_sil_policy_to_new_errc_) = rc;

    G3PLOG_E(WARNING, rc)
        << "Can't enable 'User-mode Hardware-enforced Stack Protection' "
           "process mitigation policy, app will not apply shadow stacks "
           "protection mechanism.";
  }

  if (win::GetVersion() < win::Version::WIN11_22H2) [[unlikely]] {
    // Policies below require Windows 11, version 22H2+ (Build 22621)
    std::get<std::error_code>(old_upa_policy_to_new_errc_) =
        std2::system_last_error_code(ERROR_NOT_SUPPORTED);
    std::get<std::error_code>(old_sop_policy_to_new_errc_) =
        std2::system_last_error_code(ERROR_NOT_SUPPORTED);
    return;
  }

  rc = GetProcessMitigationPolicy(
      current_process, std::get<PROCESS_MITIGATION_USER_POINTER_AUTH_POLICY>(
                           old_upa_policy_to_new_errc_));
  if (!rc) {
    const auto& old_policy =
        std::get<PROCESS_MITIGATION_USER_POINTER_AUTH_POLICY>(
            old_upa_policy_to_new_errc_);
    if (!old_policy.EnablePointerAuthUserIp) {
      PROCESS_MITIGATION_USER_POINTER_AUTH_POLICY new_policy{
          .EnablePointerAuthUserIp = TRUE};

      if ((std::get<std::error_code>(old_upa_policy_to_new_errc_) =
              SetProcessMitigationPolicy(new_policy))) {
        error_code_ = std::get<std::error_code>(old_upa_policy_to_new_errc_);

        G3PLOG_E(WARNING, error_code_)
            << "Can't enable 'User Pointer Authentication' process mitigation "
               "policy, app will not apply user pointer authentication "
               "mechanism.";
      }
    }
  } else {
    std::get<std::error_code>(old_upa_policy_to_new_errc_) = rc;

    G3PLOG_E(WARNING, rc)
        << "Can't enable 'User Pointer Authentication' process mitigation "
           "policy, app will not apply user pointer authentication "
           "mechanism.";
  }

  rc = GetProcessMitigationPolicy(
      current_process,
      std::get<PROCESS_MITIGATION_SEHOP_POLICY>(old_sop_policy_to_new_errc_));
  if (!rc) {
    const auto& old_policy =
        std::get<PROCESS_MITIGATION_SEHOP_POLICY>(old_sop_policy_to_new_errc_);
    if (!old_policy.EnableSehop) {
      PROCESS_MITIGATION_SEHOP_POLICY new_policy{.EnableSehop = TRUE};

      if ((std::get<std::error_code>(old_sop_policy_to_new_errc_) =
              SetProcessMitigationPolicy(new_policy))) {
        error_code_ = std::get<std::error_code>(old_sop_policy_to_new_errc_);

        G3PLOG_E(WARNING, error_code_)
            << "Can't enable 'Structured Exception Handling Overwrite "
               "Protection' process mitigation policy, app will not apply SEH "
               "overwrite protection mechanism.";
      }
    }
  } else {
    std::get<std::error_code>(old_sop_policy_to_new_errc_) = rc;

    G3PLOG_E(WARNING, rc)
        << "Can't enable 'Structured Exception Handling Overwrite "
           "Protection' process mitigation policy, app will not apply SEH "
           "overwrite protection mechanism.";
  }
}

ScopedProcessMitigationPolicies::ScopedProcessMitigationPoliciesImpl ::
    ~ScopedProcessMitigationPoliciesImpl() noexcept {
  {
    auto [old_policy, new_policy_result] = old_sop_policy_to_new_errc_;

    if (!new_policy_result) {
      const std::error_code rc{SetProcessMitigationPolicy(old_policy)};
      // TODO(dimhotepus): Why we can't restore original policy?
      if (rc.value() != ERROR_INVALID_PARAMETER) {
        G3PCHECK_E(!rc, rc)
            << "Can't enable 'Structured Exception Handling Overwrite "
               "Protection' process mitigation policy.";
      }
    }
  }

  {
    auto [old_policy, new_policy_result] = old_upa_policy_to_new_errc_;

    if (!new_policy_result) {
      const std::error_code rc{SetProcessMitigationPolicy(old_policy)};
      G3PCHECK_E(!rc, rc) << "Can't restore 'User Pointer Authentication' "
                             "process mitigation policy.";
    }
  }

  {
    auto [old_policy, new_policy_result] = old_uss_policy_to_new_errc_;

    if (!new_policy_result) {
      const std::error_code rc{SetProcessMitigationPolicy(old_policy)};
      G3PCHECK_E(!rc, rc)
          << "Can't restore 'User-mode Hardware-enforced Stack Protection' "
             "process mitigation policy.";
    }
  }

  {
    auto [old_policy, new_policy_result] = old_srt_policy_to_new_errc_;

    if (!new_policy_result) {
      const std::error_code rc{SetProcessMitigationPolicy(old_policy)};
      G3PCHECK_E(!rc, rc)
          << "Can't restore 'Redirection Guard' process mitigation policy.";
    }
  }

  {
    auto [old_policy, new_policy_result] = old_sil_policy_to_new_errc_;

    if (!new_policy_result) {
      const std::error_code rc{SetProcessMitigationPolicy(old_policy)};
      G3PCHECK_E(!rc, rc)
          << "Can't restore 'Strict Image Load' process mitigation policy.";
    }
  }

  {
    auto [old_policy, new_policy_result] = old_sfd_policy_to_new_errc_;

    if (!new_policy_result) {
      const std::error_code rc{SetProcessMitigationPolicy(old_policy)};
      G3PCHECK_E(!rc, rc)
          << "Can't restore 'Only system fonts' process mitigation policy.";
    }
  }

  {
    auto [old_policy, new_policy_result] = old_cfg_policy_to_new_errc_;

    if (!new_policy_result) {
      const std::error_code rc{SetProcessMitigationPolicy(old_policy)};
      G3PCHECK_E(!rc, rc)
          << "Can't restore 'DLLs must enable CFG' process mitigation policy.";
    }
  }

  {
    auto [old_policy, new_policy_result] = old_epd_policy_to_new_errc_;

    if (!new_policy_result) {
      const std::error_code rc{SetProcessMitigationPolicy(old_policy)};
      G3PCHECK_E(!rc, rc) << "Can't restore 'Extension Point Disablement' "
                             "process mitigation policy.";
    }
  }

  {
    auto [old_policy, new_policy_result] = old_shc_policy_to_new_errc_;

    if (!new_policy_result) {
      const std::error_code rc{SetProcessMitigationPolicy(old_policy)};
      G3PCHECK_E(!rc, rc) << "Can't restore 'Strict Handle Check' "
                             "process mitigation policy.";
    }
  }

  {
    auto [old_policy, new_policy_result] = old_dcp_policy_to_new_errc_;

    if (!new_policy_result) {
      const std::error_code rc{SetProcessMitigationPolicy(old_policy)};
      G3PCHECK_E(!rc, rc) << "Can't restore 'Dynamic Code Prohibition' "
                             "process mitigation policy.";
    }
  }

  {
    auto [old_policy, new_policy_result] = old_aslr_policy_to_new_errc_;

    if (!new_policy_result) {
      const std::error_code rc{SetProcessMitigationPolicy(old_policy)};
      G3PCHECK_E(!rc, rc) << "Can't restore 'Address Space Layout "
                             "Randomization' process mitigation policy.";
    }
  }

  {
    auto [old_policy, new_policy_result] = old_dep_policy_to_new_errc_;

    if (!new_policy_result && !old_policy.Permanent) {
      const std::error_code rc{SetProcessMitigationPolicy(old_policy)};
      G3PCHECK_E(!rc, rc) << "Can't restore 'Data Execution Prevention' "
                             "process mitigation policy.";
    }
  }
}

std2::result<ScopedProcessMitigationPolicies>
ScopedProcessMitigationPolicies::New() noexcept {
  ScopedProcessMitigationPolicies policies;

  return !policies.error_code()
             ? std2::result<ScopedProcessMitigationPolicies>{std::move(
                   policies)}
             : std2::result<ScopedProcessMitigationPolicies>{
                   std::unexpect, policies.error_code()};
}

ScopedProcessMitigationPolicies::ScopedProcessMitigationPolicies() noexcept
    : impl_{std::make_unique<ScopedProcessMitigationPoliciesImpl>()} {}

[[nodiscard]] std::error_code ScopedProcessMitigationPolicies::error_code()
    const noexcept {
  return impl_->error_code();
}

ScopedProcessMitigationPolicies::ScopedProcessMitigationPolicies(
    ScopedProcessMitigationPolicies&& p) noexcept = default;

ScopedProcessMitigationPolicies::~ScopedProcessMitigationPolicies() noexcept =
    default;

}  // namespace wb::base::win::security
