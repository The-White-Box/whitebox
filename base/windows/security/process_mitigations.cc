// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.

#include "process_mitigations.h"

#include "base/deps/g3log/g3log.h"
#include "base/std_ext/cstring_ext.h"
#include "base/windows/system_error_ext.h"
#include "base/windows/windows_light.h"
#include "base/windows/windows_version.h"

namespace wb::base::windows::security {
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
  return GetErrorCode(::SetDefaultDllDirectories(
      LOAD_LIBRARY_SEARCH_SYSTEM32 | LOAD_LIBRARY_SEARCH_USER_DIRS));
}

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
    : std::is_same_v<TPolicy, PROCESS_MITIGATION_USER_SHADOW_STACK_POLICY>
        ? ProcessUserShadowStackPolicy
        : MaxProcessMitigationPolicy;

template <typename TPolicy, typename R>
using mitigation_policy_concept = std::enable_if_t<
    mitigation_policy_flag<TPolicy> != MaxProcessMitigationPolicy, R>;

template <typename TPolicy>
[[nodiscard]] mitigation_policy_concept<TPolicy, std::error_code>
GetProcessMitigationPolicy(HANDLE process, TPolicy& policy) noexcept {
  std_ext::BitwiseMemset(policy, 0);

  return GetErrorCode(::GetProcessMitigationPolicy(
      process, mitigation_policy_flag<TPolicy>, &policy, sizeof(policy)));
}

template <typename TPolicy>
[[nodiscard]] mitigation_policy_concept<TPolicy, std::error_code>
SetProcessMitigationPolicy(TPolicy& policy) noexcept {
  const std::error_code rc{GetErrorCode(::SetProcessMitigationPolicy(
      mitigation_policy_flag<TPolicy>, &policy, sizeof(policy)))};

  // Access denied is ok result.
  return !rc || rc.value() == ERROR_ACCESS_DENIED ? std::error_code{} : rc;
}

ScopedProcessMitigationPolicies::ScopedProcessMitigationPolicies() noexcept
    : error_code_{},
      old_dep_policy_to_new_errc_{},
      old_aslr_policy_to_new_errc_{},
      old_dcp_policy_to_new_errc_{},
      old_shc_policy_to_new_errc_{},
      old_epd_policy_to_new_errc_{},
      old_cfg_policy_to_new_errc_{},
      old_sfd_policy_to_new_errc_{},
      old_sil_policy_to_new_errc_{},
      old_uss_policy_to_new_errc_{} {
  const HANDLE current_process{::GetCurrentProcess()};

  if (!GetProcessMitigationPolicy(current_process,
                                  std::get<PROCESS_MITIGATION_DEP_POLICY>(
                                      old_dep_policy_to_new_errc_))) {
    const auto& old_policy =
        std::get<PROCESS_MITIGATION_DEP_POLICY>(old_dep_policy_to_new_errc_);

    if (!old_policy.Permanent &&
        (!old_policy.Enable || !old_policy.DisableAtlThunkEmulation)) {
      PROCESS_MITIGATION_DEP_POLICY new_policy{.Enable = 1,
                                               .DisableAtlThunkEmulation = 1};

      if (std::get<std::error_code>(old_dep_policy_to_new_errc_) =
              SetProcessMitigationPolicy(new_policy)) {
        error_code_ = std::get<std::error_code>(old_dep_policy_to_new_errc_);

        G3LOG(WARNING)
            << "Can't enable DEP process mitigation policy, app "
               "will run with no/partial data execution prevention: "
            << std::get<std::error_code>(old_dep_policy_to_new_errc_).message();
      }
    }
  }

  if (!GetProcessMitigationPolicy(current_process,
                                  std::get<PROCESS_MITIGATION_ASLR_POLICY>(
                                      old_aslr_policy_to_new_errc_))) {
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

      if (std::get<std::error_code>(old_aslr_policy_to_new_errc_) =
              SetProcessMitigationPolicy(new_policy)) {
        error_code_ = std::get<std::error_code>(old_aslr_policy_to_new_errc_);

        G3LOG(WARNING)
            << "Can't enable ASLR process mitigation policy, app "
               "will run with no/partial address space layout randomization: "
            << std::get<std::error_code>(old_aslr_policy_to_new_errc_)
                   .message();
      }
    }
  }

  if (!GetProcessMitigationPolicy(
          current_process, std::get<PROCESS_MITIGATION_DYNAMIC_CODE_POLICY>(
                               old_dcp_policy_to_new_errc_))) {
    const auto& old_policy = std::get<PROCESS_MITIGATION_DYNAMIC_CODE_POLICY>(
        old_dcp_policy_to_new_errc_);

    if (!old_policy.ProhibitDynamicCode || old_policy.AllowThreadOptOut ||
        old_policy.AllowRemoteDowngrade) {
      PROCESS_MITIGATION_DYNAMIC_CODE_POLICY new_policy{
          .ProhibitDynamicCode = 1,
          .AllowThreadOptOut = 0,
          .AllowRemoteDowngrade = 0};

      if (std::get<std::error_code>(old_dcp_policy_to_new_errc_) =
              SetProcessMitigationPolicy(new_policy)) {
        error_code_ = std::get<std::error_code>(old_dcp_policy_to_new_errc_);

        G3LOG(WARNING)
            << "Can't enable 'Dynamic Code' process mitigation policy, app "
               "will run with no/partial dynamic code prohibition: "
            << std::get<std::error_code>(old_dcp_policy_to_new_errc_).message();
      }
    }
  }

  if (!GetProcessMitigationPolicy(
          current_process,
          std::get<PROCESS_MITIGATION_STRICT_HANDLE_CHECK_POLICY>(
              old_shc_policy_to_new_errc_))) {
    const auto& old_policy =
        std::get<PROCESS_MITIGATION_STRICT_HANDLE_CHECK_POLICY>(
            old_shc_policy_to_new_errc_);

    if (!old_policy.RaiseExceptionOnInvalidHandleReference ||
        !old_policy.HandleExceptionsPermanentlyEnabled) {
      PROCESS_MITIGATION_STRICT_HANDLE_CHECK_POLICY new_policy{
          .RaiseExceptionOnInvalidHandleReference = 1,
          .HandleExceptionsPermanentlyEnabled = 1};

      if (std::get<std::error_code>(old_shc_policy_to_new_errc_) =
              SetProcessMitigationPolicy(new_policy)) {
        error_code_ = std::get<std::error_code>(old_shc_policy_to_new_errc_);

        G3LOG(WARNING)
            << "Can't enable 'Strict Handle Check' process mitigation policy, "
               "app will run with no/partial strict handle checking: "
            << std::get<std::error_code>(old_shc_policy_to_new_errc_).message();
      }
    }
  }

  if (!GetProcessMitigationPolicy(
          current_process,
          std::get<PROCESS_MITIGATION_EXTENSION_POINT_DISABLE_POLICY>(
              old_epd_policy_to_new_errc_))) {
    const auto& old_policy =
        std::get<PROCESS_MITIGATION_EXTENSION_POINT_DISABLE_POLICY>(
            old_epd_policy_to_new_errc_);

    if (!old_policy.DisableExtensionPoints) {
      PROCESS_MITIGATION_EXTENSION_POINT_DISABLE_POLICY new_policy{
          .DisableExtensionPoints = 1};

      if (std::get<std::error_code>(old_epd_policy_to_new_errc_) =
              SetProcessMitigationPolicy(new_policy)) {
        error_code_ = std::get<std::error_code>(old_epd_policy_to_new_errc_);

        G3LOG(WARNING)
            << "Can't enable 'Extension Point Disablement' process mitigation "
               "policy, app will run with not disabled extension points: "
            << std::get<std::error_code>(old_epd_policy_to_new_errc_).message();
      }
    }
  }

  if (!GetProcessMitigationPolicy(
          current_process,
          std::get<PROCESS_MITIGATION_CONTROL_FLOW_GUARD_POLICY>(
              old_cfg_policy_to_new_errc_))) {
    const auto& old_policy =
        std::get<PROCESS_MITIGATION_CONTROL_FLOW_GUARD_POLICY>(
            old_cfg_policy_to_new_errc_);
    // Only known option to be altered via API for now.
    if (!old_policy.StrictMode) {
      PROCESS_MITIGATION_CONTROL_FLOW_GUARD_POLICY new_policy{.StrictMode =
                                                                  TRUE};

      if (std::get<std::error_code>(old_cfg_policy_to_new_errc_) =
              SetProcessMitigationPolicy(new_policy)) {
        error_code_ = std::get<std::error_code>(old_cfg_policy_to_new_errc_);

        G3LOG(WARNING)
            << "Can't enable 'DLLs must enable CFG' process mitigation "
               "policy, app will run with DLLs which not enable CFG: "
            << std::get<std::error_code>(old_cfg_policy_to_new_errc_).message();
      }
    }
  }

  if (!GetProcessMitigationPolicy(
          current_process, std::get<PROCESS_MITIGATION_FONT_DISABLE_POLICY>(
                               old_sfd_policy_to_new_errc_))) {
    const auto& old_policy = std::get<PROCESS_MITIGATION_FONT_DISABLE_POLICY>(
        old_sfd_policy_to_new_errc_);
    if (!old_policy.DisableNonSystemFonts) {
      PROCESS_MITIGATION_FONT_DISABLE_POLICY new_policy{.DisableNonSystemFonts =
                                                            TRUE};

      if (std::get<std::error_code>(old_sfd_policy_to_new_errc_) =
              SetProcessMitigationPolicy(new_policy)) {
        error_code_ = std::get<std::error_code>(old_sfd_policy_to_new_errc_);

        G3LOG(WARNING)
            << "Can't enable 'Only system fonts' process mitigation "
               "policy, app will run with custom fonts: "
            << std::get<std::error_code>(old_sfd_policy_to_new_errc_).message();
      }
    }
  }

  if (!GetProcessMitigationPolicy(
          current_process, std::get<PROCESS_MITIGATION_IMAGE_LOAD_POLICY>(
                               old_sil_policy_to_new_errc_))) {
    const auto& old_policy = std::get<PROCESS_MITIGATION_IMAGE_LOAD_POLICY>(
        old_sil_policy_to_new_errc_);
    if (!old_policy.NoRemoteImages || !old_policy.NoLowMandatoryLabelImages ||
        !old_policy.PreferSystem32Images) {
      PROCESS_MITIGATION_IMAGE_LOAD_POLICY new_policy{
          .NoRemoteImages = TRUE,
          .NoLowMandatoryLabelImages = TRUE,
          .PreferSystem32Images = TRUE};

      if (std::get<std::error_code>(old_sil_policy_to_new_errc_) =
              SetProcessMitigationPolicy(new_policy)) {
        error_code_ = std::get<std::error_code>(old_sil_policy_to_new_errc_);

        G3LOG(WARNING)
            << "Can't enable 'Strict Image Load' process mitigation "
               "policy, app will load images from unsafe locations: "
            << std::get<std::error_code>(old_sil_policy_to_new_errc_).message();
      }
    }
  }

  if (windows::GetVersion() < windows::Version::WIN10_20H1) [[unlikely]] {
    // Policies below require Windows 10, version 2004+ (Build 19041)
    std::get<std::error_code>(old_uss_policy_to_new_errc_) =
        std::error_code(ERROR_NOT_SUPPORTED, std::system_category());
    return;
  }

  if (!GetProcessMitigationPolicy(
          current_process,
          std::get<PROCESS_MITIGATION_USER_SHADOW_STACK_POLICY>(
              old_uss_policy_to_new_errc_))) {
    const auto& old_policy =
        std::get<PROCESS_MITIGATION_USER_SHADOW_STACK_POLICY>(
            old_uss_policy_to_new_errc_);
    if (!old_policy.EnableUserShadowStack || !old_policy.BlockNonCetBinaries) {
      PROCESS_MITIGATION_USER_SHADOW_STACK_POLICY new_policy{
          .EnableUserShadowStack = TRUE, .BlockNonCetBinaries = TRUE};

      if (std::get<std::error_code>(old_uss_policy_to_new_errc_) =
              SetProcessMitigationPolicy(new_policy)) {
        error_code_ = std::get<std::error_code>(old_uss_policy_to_new_errc_);

        G3LOG(WARNING)
            << "Can't enable 'User-mode Hardware-enforced Stack Protection' "
               "process mitigation policy, app will not apply shadow stacks "
               "protection mechanism: "
            << std::get<std::error_code>(old_uss_policy_to_new_errc_).message();
      }
    }
  }
}

ScopedProcessMitigationPolicies::~ScopedProcessMitigationPolicies() noexcept {
  {
    auto [old_policy, new_policy_result] = old_uss_policy_to_new_errc_;

    if (!new_policy_result) {
      G3CHECK(!SetProcessMitigationPolicy(old_policy));
    }
  }

  {
    auto [old_policy, new_policy_result] = old_sil_policy_to_new_errc_;

    if (!new_policy_result) {
      G3CHECK(!SetProcessMitigationPolicy(old_policy));
    }
  }

  {
    auto [old_policy, new_policy_result] = old_sfd_policy_to_new_errc_;

    if (!new_policy_result) {
      G3CHECK(!SetProcessMitigationPolicy(old_policy));
    }
  }

  {
    auto [old_policy, new_policy_result] = old_cfg_policy_to_new_errc_;

    if (!new_policy_result) {
      G3CHECK(!SetProcessMitigationPolicy(old_policy));
    }
  }

  {
    auto [old_policy, new_policy_result] = old_epd_policy_to_new_errc_;

    if (!new_policy_result) {
      G3CHECK(!SetProcessMitigationPolicy(old_policy));
    }
  }

  {
    auto [old_policy, new_policy_result] = old_shc_policy_to_new_errc_;

    if (!new_policy_result) {
      G3CHECK(!SetProcessMitigationPolicy(old_policy));
    }
  }

  {
    auto [old_policy, new_policy_result] = old_dcp_policy_to_new_errc_;

    if (!new_policy_result) {
      G3CHECK(!SetProcessMitigationPolicy(old_policy));
    }
  }

  {
    auto [old_policy, new_policy_result] = old_aslr_policy_to_new_errc_;

    if (!new_policy_result) {
      G3CHECK(!SetProcessMitigationPolicy(old_policy));
    }
  }

  {
    auto [old_policy, new_policy_result] = old_dep_policy_to_new_errc_;

    if (!new_policy_result && !old_policy.Permanent) {
      G3CHECK(!SetProcessMitigationPolicy(old_policy));
    }
  }
}
}  // namespace wb::base::windows::security
