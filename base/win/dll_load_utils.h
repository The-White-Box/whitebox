// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// DLL load utils.

#ifndef WB_BASE_WIN_DLL_LOAD_UTILS_H_
#define WB_BASE_WIN_DLL_LOAD_UTILS_H_

#include <sal.h>
#include <winerror.h>  // ERROR_INSUFFICIENT_BUFFER

#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <string_view>

#include "base/base_switches.h"
#include "base/std2/system_error_ext.h"
#include "build/compiler_config.h"

using HINSTANCE = struct HINSTANCE__*;
using HMODULE = HINSTANCE;

extern "C" WB_ATTRIBUTE_DLL_IMPORT _Success_(return != 0)
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
    _In_ std::string_view command_line) noexcept {
  const size_t insecure_arg_idx{
      command_line.find(switches::insecure::kAllowUnsignedModuleTargetFlag)};
  // No arg.
  if (insecure_arg_idx == std::string_view::npos) [[likely]] {
    return true;
  }

  // Should start with arg or has space char before.
  if (insecure_arg_idx != 0U && !std::isspace(static_cast<unsigned char>(
                                    command_line[insecure_arg_idx - 1U]))) {
    return true;
  }

  // Should end with arg or has space char after.
  const size_t next_char_after_insecure_arg_idx{
      insecure_arg_idx +
      sizeof(switches::insecure::kAllowUnsignedModuleTargetFlag) - 1U};
  return next_char_after_insecure_arg_idx < command_line.size() &&
         !std::isspace(static_cast<unsigned char>(
             command_line[next_char_after_insecure_arg_idx]));
}

/**
 * @brief Get app directory.
 * @param instance App instance.
 * @return App directory with trailing path separator.
 */
std2::result<std::string> GetApplicationDirectory(_In_ HINSTANCE instance) {
  std::string file_path;
  file_path.resize(_MAX_PATH + 1);

  const unsigned long file_name_path_size{
      ::GetModuleFileNameA(instance, file_path.data(),
                           static_cast<unsigned long>(file_path.size()))};
  if (file_name_path_size != 0) {
    if (std2::GetThreadNativeLastErrno() == ERROR_INSUFFICIENT_BUFFER) {
      return std::error_code{ERROR_INSUFFICIENT_BUFFER, std::system_category()};
    }

    file_path.resize(file_name_path_size);

    const size_t last_separator_pos{
        file_path.rfind(std::filesystem::path::preferred_separator)};
    return last_separator_pos != std::wstring::npos
               ? file_path.substr(0, last_separator_pos + 1)
               : file_path;
  }

  return std2::GetThreadErrorCode();
}
}  // namespace wb::base::windows

#endif  // !WB_BASE_WIN_DLL_LOAD_UTILS_H_
