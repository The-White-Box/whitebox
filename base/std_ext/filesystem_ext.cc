// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// <filesystem> extensions.

#include "filesystem_ext.h"

#include <unistd.h>

#include "base/std_ext/system_error_ext.h"

namespace wb::base::std_ext {
/**
 * Gets path to invoking executable.
 * @tparam path_size Path size.
 * @tparam max_path_size Max path size.
 * @param rc Error code.
 * @return Path to invoking executable.
 */
template <size_t path_size = 128U, size_t max_path_size = 8192U>
[[nodiscard]] inline std::filesystem::path GetExecutablePath(
    std::error_code& rc) noexcept {
#ifdef WB_OS_POSIX
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
  rc = wb::base::std_ext::GetThreadErrorCode();
  return std::filesystem::path{};
#else
#error Please add get executable path API for your OS.
#endif
}

/**
 * Gets path to invoking executable directory.
 * @param rc Error code.
 * @return Path to executable directory.
 */
[[nodiscard]] WB_BASE_API std::filesystem::path GetExecutableDirectory(
    std::error_code& rc) noexcept {
  std::filesystem::path exe_path{GetExecutablePath(rc)};

  return exe_path.parent_path();
}
}  // namespace wb::base::std_ext
