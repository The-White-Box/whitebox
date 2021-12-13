// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// <system_error> extensions.

#include "system_error_ext.h"
//
#ifdef WB_OS_WIN
#include "base/win/windows_light.h"
#endif

#include "base/deps/googletest/gtest/gtest.h"

using namespace wb::base;

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(SystemErrorExtTest, native_last_errno) {
#ifdef WB_OS_WIN
  ::SetLastError(ERROR_ACCESS_DENIED);
  EXPECT_EQ(ERROR_ACCESS_DENIED, std2::native_last_errno());

  ::SetLastError(0);
  EXPECT_EQ(0, std2::native_last_errno());
#else
  errno = EPERM;
  EXPECT_EQ(EPERM, std2::native_last_errno());

  errno = 0;
  EXPECT_EQ(0, std2::native_last_errno());
#endif

#ifdef WB_OS_WIN
  std2::native_last_errno(std::error_code(static_cast<int>(ERROR_ACCESS_DENIED),
                                          std::system_category()));
  EXPECT_EQ(std::error_code(static_cast<int>(ERROR_ACCESS_DENIED),
                            std::system_category()),
            std2::system_last_error_code());

  EXPECT_EQ(
      std::error_code(static_cast<int>(ERROR_RETRY), std::system_category()),
      std2::system_last_error_code(ERROR_RETRY));

  std2::native_last_errno(std::error_code());
  EXPECT_EQ(std::error_code(), std2::system_last_error_code());
#else
  std2::native_last_errno(std::error_code(EPERM, std::system_category()));
  EXPECT_EQ(EPERM, std2::native_last_errno());

  EXPECT_EQ(std::error_code(EAGAIN, std::system_category()),
            std2::system_last_error_code(EAGAIN));

  std2::native_last_errno(std::error_code(0, std::system_category()));
  EXPECT_EQ(0, std2::native_last_errno());
#endif
}

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(SystemErrorExtTest, posix_last_error_code) {
  errno = EPERM;
  EXPECT_EQ(std::error_code(EPERM, std::generic_category()),
            std2::posix_last_error_code());

  EXPECT_EQ(std::error_code(EAGAIN, std::generic_category()),
            std2::posix_last_error_code(EAGAIN));

  errno = 0;
  EXPECT_EQ(std::error_code(0, std::generic_category()),
            std2::posix_last_error_code());
}

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(SystemErrorExtTest, system_last_error_code) {
#ifdef WB_OS_WIN
  ::SetLastError(ERROR_ACCESS_DENIED);
  EXPECT_EQ(std::error_code(static_cast<int>(ERROR_ACCESS_DENIED),
                            std::system_category()),
            std2::system_last_error_code());

  EXPECT_EQ(
      std::error_code(static_cast<int>(ERROR_RETRY), std::system_category()),
      std2::system_last_error_code(ERROR_RETRY));

  ::SetLastError(0);
  EXPECT_EQ(std::error_code(), std2::system_last_error_code());
#else
  errno = EPERM;
  EXPECT_EQ(EPERM, std2::native_last_errno());

  EXPECT_EQ(std::error_code(EAGAIN, std::system_category()),
            std2::system_last_error_code(EAGAIN));

  errno = 0;
  EXPECT_EQ(0, std2::native_last_errno());
#endif
}

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(SystemErrorExtTest, get_error) {
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
  std2::result<int> r1{12};
  EXPECT_EQ(nullptr, std2::get_error(r1));

  const auto rc = std::error_code{EAGAIN, std::generic_category()};
  const std2::result<int> r2{rc};
  EXPECT_NE(nullptr, std2::get_error(r2));
  EXPECT_EQ(rc, *std2::get_error(r2));
}

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(SystemErrorExtTest, get_result) {
  const auto rc = std::error_code{EAGAIN, std::generic_category()};
  const std2::result<int> r1{rc};
  EXPECT_EQ(nullptr, std2::get_result(r1));

  // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
  std2::result<int> r2{12};
  EXPECT_EQ(12, *std2::get_result(r2));

  const std2::result<int> r3{57};
  EXPECT_EQ(57, *std2::get_result(r3));
}
