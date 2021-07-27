// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// DLL load utils.

#ifndef WB_BASE_WINDOWS_DLL_LOAD_UTILS_H_
#define WB_BASE_WINDOWS_DLL_LOAD_UTILS_H_

#include <sal.h>
#include <winerror.h>  // ERROR_INSUFFICIENT_BUFFER

#include <cstring>

#include "base/std_ext/system_error_ext.h"
#include "build/command_line_flags.h"

struct HINSTANCE__;

/**
 * @brief HINSTANCE type.
 */
using HINSTANCE = struct HINSTANCE__*;

/**
 * @brief HMODULE type.
 */
using HMODULE = HINSTANCE;

WINBASEAPI
_Success_(return != 0)
    _Ret_range_(1, nSize) unsigned long __stdcall GetModuleFileNameA(
        _In_opt_ HMODULE hModule,
        _Out_writes_to_(nSize, ((return < nSize) ? (return +1)
                                                 : nSize)) char* lpFilename,
        _In_ unsigned long nSize);

namespace wb::base::windows {
/**
 * @brief Checks either dll to load must be signed or not.
 * @param command_line Command line.
 * @return true if signed dll required, false otherwise.
 */
[[nodiscard]] inline bool MustBeSignedDllLoadTarget(
    _In_ const char* command_line) noexcept {
  const char* unsafe_allow_unsigned_module_target{std::strstr(
      command_line, wb::build::cmd_args::kUnsafeAllowUnsignedModuleTargetFlag)};

  if (unsafe_allow_unsigned_module_target == nullptr) return true;

  const char after_flag_char{
      *(unsafe_allow_unsigned_module_target +
        sizeof(wb::build::cmd_args::kUnsafeAllowUnsignedModuleTargetFlag) - 1)};
  return after_flag_char != '\0' &&
         !std::isspace(static_cast<unsigned char>(after_flag_char));
}

/**
 * @brief Get app directory.
 * @param instance App instance.
 * @return App directory with trailing path separator.
 */
wb::base::std_ext::os_res<std::string> GetApplicationDirectory(
    _In_ HINSTANCE instance) {
  std::string file_path;
  file_path.resize(MAX_PATH);

  const DWORD file_name_path_size{::GetModuleFileNameA(
      instance, file_path.data(), static_cast<DWORD>(file_path.size()))};
  if (file_name_path_size != 0) {
    if (wb::base::std_ext::GetThreadNativeLastErrno() ==
        ERROR_INSUFFICIENT_BUFFER) {
      return std::error_code{ERROR_INSUFFICIENT_BUFFER, std::system_category()};
    }

    file_path.resize(file_name_path_size);

    const size_t last_separator_pos{
        file_path.rfind(std::filesystem::path::preferred_separator)};
    return last_separator_pos != std::wstring::npos
               ? file_path.substr(0, last_separator_pos + 1)
               : file_path;
  }

  return wb::base::std_ext::GetThreadErrorCode();
}
}  // namespace wb::base::windows

#endif  // !WB_BASE_WINDOWS_DLL_LOAD_UTILS_H_
