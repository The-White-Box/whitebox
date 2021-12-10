// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// <filesystem> extensions.

#include "filesystem_ext.h"
//
#include "base/macroses.h"
//
#include "base/deps/googletest/gtest/gtest.h"

using namespace wb::base::std2;

namespace {

/**
 * @brief Changes current path in scope.
 */
class ScopedChangeCurrentPath {
 public:
  /**
   * @brief Changes current path in scope.
   * @param new_path New current path for scope.
   */
  explicit ScopedChangeCurrentPath(const std::filesystem::path &new_path)
      : rc_{}, original_current_path_{std::filesystem::current_path(rc_)} {
    EXPECT_EQ(std::error_code{}, rc_);
    std::filesystem::current_path(new_path, rc_);
    EXPECT_EQ(std::error_code{}, rc_);
  }

  ~ScopedChangeCurrentPath() noexcept {
    if (!rc_) {
      try {
        std::filesystem::current_path(original_current_path_, rc_);
        EXPECT_EQ(std::error_code{}, rc_);
      } catch (const std::exception &ex) {
        EXPECT_FALSE(!!ex.what())
            << "~ScopedChangeCurrentPath throws " << ex.what();
      }
    }
  }

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedChangeCurrentPath);

 private:
  /**
   * @brief Error code for current path change.
   */
  std::error_code rc_;
  /**
   * @brief Original current path.
   */
  const std::filesystem::path original_current_path_;
};

}  // namespace

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(FilesystemExtTest, get_executable_directory) {
  std::error_code rc;
  auto binary_path = std::filesystem::current_path(rc);
  EXPECT_EQ(std::error_code{}, rc)
      << "Should get current path which is the same as binary path.";
  auto temp_path = std::filesystem::temp_directory_path(rc);
  EXPECT_NE(binary_path, temp_path) << "Current path is not same as temp one.";
  EXPECT_EQ(std::error_code{}, rc)
      << "Should change current path to temp path.";

  const ScopedChangeCurrentPath scoped_current_path{temp_path};

  EXPECT_EQ(wb::base::std2::result<std::filesystem::path>{binary_path},
            filesystem::get_executable_directory())
      << "Should get executable directory, not current path.";
}

#ifdef WB_OS_WIN
// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(FilesystemExtTests, get_short_exe_name_from_command_line) {
  EXPECT_EQ(std::nullopt, filesystem::get_short_exe_name_from_command_line(""));
  EXPECT_EQ(std::nullopt,
            filesystem::get_short_exe_name_from_command_line("abc"));
  EXPECT_EQ(std::nullopt,
            filesystem::get_short_exe_name_from_command_line("abc.exe"));
  EXPECT_EQ(std::nullopt, filesystem::get_short_exe_name_from_command_line(
                              "some_very_large_executable_file_to_run"));

  EXPECT_EQ(std::optional<std::string_view>{"executable.exe"},
            filesystem::get_short_exe_name_from_command_line(
                "some\\very\\long\\executable.exe"));
  EXPECT_EQ(std::optional<std::string_view>{"executable.exe"},
            filesystem::get_short_exe_name_from_command_line(
                R"("some\\very\\long\\executable.exe")"));
  EXPECT_EQ(std::optional<std::string_view>{"executable.exe"},
            filesystem::get_short_exe_name_from_command_line(
                R"("some\\very\\long\\executable.exe" --some_arg)"));
  EXPECT_EQ(
      std::optional<std::string_view>{"executable.exe"},
      filesystem::get_short_exe_name_from_command_line(
          R"("some\\very\\long\\executable.exe" --some_arg --some-other-arg)"));
  EXPECT_EQ(
      std::optional<std::string_view>{"executable.exe"},
      filesystem::get_short_exe_name_from_command_line(
          R"("some\\very\\long\\executable.exe" "--some_arg" --some-other-arg)"));
  EXPECT_EQ(std::optional<std::string_view>{"executable.exe"},
            filesystem::get_short_exe_name_from_command_line(
                R"("some\\very\\long\\executable.exe" " " ")"));
}
#endif  // !WB_OS_WIN
