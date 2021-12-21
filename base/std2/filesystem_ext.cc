// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// <filesystem> extensions.

#include "filesystem_ext.h"

#ifdef WB_OS_POSIX
#include <unistd.h>

#include <array>
#endif

#ifdef WB_OS_WIN
#include "base/win/system_error_ext.h"
#include "base/win/windows_light.h"
#endif

#include "base/std2/string_view_ext.h"
#include "base/std2/system_error_ext.h"

namespace {

#ifdef WB_OS_POSIX
/**
 * Gets path to invoking executable.
 * @tparam path_size Path size.
 * @tparam max_path_size Max path size.
 * @param rc Error code.
 * @return Path to invoking executable.
 */
// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers): Constants are good.
template <size_t path_size = 128U, size_t max_path_size = 1024U>
[[nodiscard]] inline wb::base::std2::result<std::filesystem::path>
GetExecutablePath() noexcept {
  using namespace wb::base::std2;

  std::array<char, path_size> buffer{};
  const ssize_t bytes_required{
      ::readlink("/proc/self/exe", buffer.data(), std::size(buffer))};

  if (bytes_required > 0) {
    if (bytes_required != std::size(buffer)) {
      // readlink() does not append a terminating null byte to buf.
      buffer[std::size(buffer) - 1] = '\0';
      return std::filesystem::path{buffer.data()};
    }

    // May be truncated?  Try to extend path size.
    if constexpr (path_size < max_path_size) {
      constexpr size_t new_size{std::min(path_size << 1U, max_path_size)};
      return GetExecutablePath<new_size>();
    }

    return system_last_error_code(ENOMEM);
  }

  // Error case.
  return system_last_error_code();
}
#endif

}  // namespace

namespace wb::base::std2::filesystem {

/**
 * @brief Gets path to invoking executable directory.
 * @return Path to executable directory.
 */
[[nodiscard]] WB_BASE_API result<std::filesystem::path>
get_executable_directory() noexcept {
#ifdef WB_OS_POSIX
  auto exe_path_result{GetExecutablePath()};

  if (auto *result = std2::get_result(exe_path_result)) WB_ATTRIBUTE_LIKELY {
      return result->parent_path();
    }

  return exe_path_result;
#elif defined(WB_OS_WIN)
  std::string file_path;
  file_path.resize(_MAX_PATH + 1);

  const unsigned long file_name_path_size{
      ::GetModuleFileNameA(::GetModuleHandleA(nullptr), file_path.data(),
                           static_cast<unsigned long>(file_path.size()))};
  if (file_name_path_size != 0) {
    if (native_last_errno() == ERROR_INSUFFICIENT_BUFFER) {
      return std2::system_last_error_code(ERROR_INSUFFICIENT_BUFFER);
    }

    file_path.resize(file_name_path_size);

    const size_t last_separator_pos{
        file_path.rfind(std::filesystem::path::preferred_separator)};
    return std::filesystem::path{last_separator_pos != std::wstring::npos
                                     ? file_path.substr(0, last_separator_pos)
                                     : file_path};
  }

  return system_last_error_code();
#else
#error "Please define get_executable_directory for your OS."
#endif
}

#ifdef WB_OS_WIN
/**
 * @brief Extract short exe name from command line.
 * @param command_line Command line.
 * @return Short exe name.
 */
[[nodiscard]] WB_BASE_API std::optional<std::string_view>
get_short_exe_name_from_command_line(std::string_view command_line) noexcept {
  // Sometimes /foo/bla is also passed on windows, ex. by Visual Studio g3log
  // tests discovery.
  constexpr char native_path_separators[]{"\\/"};

  // Assume "x:\zzzzz\yyyy.exe" www on Windows.
  if (std2::starts_with(command_line, '"')) {
    const size_t end_exe_double_quote_idx{command_line.find('"', 1U)};
    const size_t separator_before_name_idx{command_line.find_last_of(
        native_path_separators, end_exe_double_quote_idx)};

    if (end_exe_double_quote_idx != std::string_view::npos &&
        separator_before_name_idx != std::string_view::npos) {
      return command_line.substr(
          separator_before_name_idx + 1,
          end_exe_double_quote_idx - separator_before_name_idx - 1);
    }
  }

  // Has path separators.
  if (const size_t separator_before_name_idx{
          command_line.find_last_of(native_path_separators)};
      separator_before_name_idx != std::string_view::npos) {
    return command_line.substr(
        separator_before_name_idx + 1,
        command_line.size() - separator_before_name_idx - 1);
  }

  return {};
}
#endif  // WB_OS_WIN

}  // namespace wb::base::std2::filesystem
