// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// DLL load utils.

#ifndef WB_BASE_WIN_DLL_LOAD_UTILS_H_
#define WB_BASE_WIN_DLL_LOAD_UTILS_H_

#include <sal.h>
#include <winerror.h>  // ERROR_INSUFFICIENT_BUFFER

#include <filesystem>
#include <string>

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

namespace wb::base::win {

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
    if (std2::native_last_errno() == ERROR_INSUFFICIENT_BUFFER) {
      return std2::system_last_error_code(ERROR_INSUFFICIENT_BUFFER);
    }

    file_path.resize(file_name_path_size);

    const size_t last_separator_pos{
        file_path.rfind(std::filesystem::path::preferred_separator)};
    return last_separator_pos != std::wstring::npos
               ? file_path.substr(0, last_separator_pos + 1)
               : file_path;
  }

  return std2::system_last_error_code();
}

}  // namespace wb::base::win

#endif  // !WB_BASE_WIN_DLL_LOAD_UTILS_H_
