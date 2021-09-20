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

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(SystemErrorExtTests, GetThreadNativeLastErrno) {
#ifdef WB_OS_WIN
  ::SetLastError(ERROR_ACCESS_DENIED);
  EXPECT_EQ(ERROR_ACCESS_DENIED, std2::GetThreadNativeLastErrno());

  ::SetLastError(0);
  EXPECT_EQ(0, std2::GetThreadNativeLastErrno());
#else
  errno = EPERM;
  EXPECT_EQ(EPERM, std2::GetThreadNativeLastErrno());

  errno = 0;
  EXPECT_EQ(0, std2::GetThreadNativeLastErrno());
#endif
}

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(SystemErrorExtTests, GetThreadPosixErrorCode) {
  errno = EPERM;
  EXPECT_EQ(std::error_code(EPERM, std::generic_category()),
            std2::GetThreadPosixErrorCode());

  EXPECT_EQ(std::error_code(EAGAIN, std::generic_category()),
            std2::GetThreadPosixErrorCode(EAGAIN));

  errno = 0;
  EXPECT_EQ(std::error_code(0, std::generic_category()),
            std2::GetThreadPosixErrorCode());
}

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(SystemErrorExtTests, GetThreadErrorCode) {
#ifdef WB_OS_WIN
  ::SetLastError(ERROR_ACCESS_DENIED);
  EXPECT_EQ(std::error_code(static_cast<int>(ERROR_ACCESS_DENIED),
                            std::system_category()),
            std2::GetThreadErrorCode());

  EXPECT_EQ(
      std::error_code(static_cast<int>(ERROR_RETRY), std::system_category()),
      std2::GetThreadErrorCode(ERROR_RETRY));

  ::SetLastError(0);
  EXPECT_EQ(std::error_code(), std2::GetThreadErrorCode());
#else
  errno = EPERM;
  EXPECT_EQ(std::error_code(EPERM, std::system_category()),
            std2::GetThreadNativeLastErrno());

  EXPECT_EQ(std::error_code(EAGAIN, std::system_category()),
            std2::GetThreadErrorCode(EAGAIN));

  errno = 0;
  EXPECT_EQ(std::error_code(), std2::GetThreadNativeLastErrno());
#endif
}

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(SystemErrorExtTests, SetThreadErrorCode) {
#ifdef WB_OS_WIN
  std2::SetThreadErrorCode(std::error_code(
      static_cast<int>(ERROR_ACCESS_DENIED), std::system_category()));
  EXPECT_EQ(std::error_code(static_cast<int>(ERROR_ACCESS_DENIED),
                            std::system_category()),
            std2::GetThreadErrorCode());

  EXPECT_EQ(
      std::error_code(static_cast<int>(ERROR_RETRY), std::system_category()),
      std2::GetThreadErrorCode(ERROR_RETRY));

  std2::SetThreadErrorCode(std::error_code());
  EXPECT_EQ(std::error_code(), std2::GetThreadErrorCode());
#else
  std2::SetThreadErrorCode(std::error_code(EPERM, std::system_category()));
  EXPECT_EQ(std::error_code(EPERM, std::system_category()),
            std2::GetThreadNativeLastErrno());

  EXPECT_EQ(std::error_code(EAGAIN, std::system_category()),
            std2::GetThreadErrorCode(EAGAIN));

  std2::SetThreadErrorCode(std::error_code(0, std::system_category()));
  EXPECT_EQ(std::error_code(), std2::GetThreadNativeLastErrno());
#endif
}

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(SystemErrorExtTests, GetErrorCode) {
  std2::result<int> r1{12};
  EXPECT_EQ(nullptr, std2::GetErrorCode(r1));

  const auto rc = std::error_code{EAGAIN, std::generic_category()};
  const std2::result<int> r2{rc};
  EXPECT_NE(nullptr, std2::GetErrorCode(r2));
  EXPECT_EQ(rc, *std2::GetErrorCode(r2));
}

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(SystemErrorExtTests, GetSuccessResult) {
  const auto rc = std::error_code{EAGAIN, std::generic_category()};
  const std2::result<int> r1{rc};
  EXPECT_EQ(nullptr, std2::GetSuccessResult(r1));

  std2::result<int> r2{12};
  EXPECT_EQ(12, *std2::GetSuccessResult(r2));

  const std2::result<int> r3{57};
  EXPECT_EQ(57, *std2::GetSuccessResult(r3));
}
