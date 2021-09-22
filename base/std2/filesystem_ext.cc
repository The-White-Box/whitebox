// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// <filesystem> extensions.

#include "filesystem_ext.h"

#ifdef WB_OS_POSIX
#include <unistd.h>
#endif

#ifdef WB_OS_WIN
#include "base/win/system_error_ext.h"
#include "base/win/windows_light.h"
#endif

#include "base/std2/system_error_ext.h"

namespace wb::base::std2 {
#ifdef WB_OS_POSIX
/**
 * Gets path to invoking executable.
 * @tparam path_size Path size.
 * @tparam max_path_size Max path size.
 * @param rc Error code.
 * @return Path to invoking executable.
 */
template <size_t path_size = 128U, size_t max_path_size = 1024U>
[[nodiscard]] inline std::filesystem::path GetExecutablePath(
    std::error_code& rc) noexcept {
  char buffer[path_size];
  const ssize_t bytes_required{
      ::readlink("/proc/self/exe", buffer, std::size(buffer))};

  if (bytes_required > 0) {
    if (bytes_required != std::size(buffer)) {
      // readlink() does not append a terminating null byte to buf.
      buffer[std::size(buffer) - 1] = '\0';
      return std::filesystem::path{buffer};
    }

    // May be truncated?  Try to extend path size.
    return GetExecutablePath<std::min(path_size << 1U, max_path_size)>(rc);
  }

  // Error case.
  rc = GetThreadErrorCode();
  return std::filesystem::path{};
}
#endif

/**
 * Gets path to invoking executable directory.
 * @param rc Error code.
 * @return Path to executable directory.
 */
[[nodiscard]] WB_BASE_API std::filesystem::path GetExecutableDirectory(
    std::error_code& rc) noexcept {
#ifdef WB_OS_POSIX
  std::filesystem::path exe_path{GetExecutablePath(rc)};

  return exe_path.parent_path();
#elif defined(WB_OS_WIN)
  std::string file_path;
  file_path.resize(_MAX_PATH + 1);

  const unsigned long file_name_path_size{
      ::GetModuleFileNameA(::GetModuleHandleA(nullptr), file_path.data(),
                           static_cast<unsigned long>(file_path.size()))};
  if (file_name_path_size != 0) {
    if (GetThreadNativeLastErrno() == ERROR_INSUFFICIENT_BUFFER) {
      rc = std::error_code{implicit_cast<int>(ERROR_INSUFFICIENT_BUFFER),
                           std::system_category()};
    }

    file_path.resize(file_name_path_size);

    const size_t last_separator_pos{
        file_path.rfind(std::filesystem::path::preferred_separator)};
    return std::filesystem::path{
        last_separator_pos != std::wstring::npos
            ? file_path.substr(0, last_separator_pos + 1)
            : file_path};
  }

  rc = GetThreadErrorCode();
  return std::filesystem::path{};
#else
#error Please define GetExecutableDirectory for your OS.
#endif
}
}  // namespace wb::base::std_ext
