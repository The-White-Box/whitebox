// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// HANDLE RAII wrapper.

#include "unique_handle.h"
//
#include "windows_light.h"
//
#include "base/deps/googletest/gtest/gtest.h"

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(UniqueHandleTest, ShouldCheckHandleIsValid) {
  EXPECT_FALSE(static_cast<const bool>(wb::base::win::unique_handle(nullptr)));
  EXPECT_FALSE(static_cast<const bool>(
      wb::base::win::unique_handle(INVALID_HANDLE_VALUE)));
  EXPECT_TRUE(
      static_cast<const bool>(wb::base::win::unique_handle(::OpenProcess(
          PROCESS_QUERY_INFORMATION, FALSE, ::GetCurrentProcessId()))));
}

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(UniqueHandleTest, ShouldGetNativeHandle) {
  EXPECT_EQ(nullptr, wb::base::win::unique_handle(nullptr).get());
  EXPECT_EQ(wb::base::win::kInvalidNativeHandle,
            wb::base::win::unique_handle(INVALID_HANDLE_VALUE).get());
  EXPECT_NE(nullptr, wb::base::win::unique_handle(
                         ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE,
                                       ::GetCurrentProcessId()))
                         .get());
}

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(UniqueHandleTest, ShouldResetHandle) {
  auto handle = wb::base::win::unique_handle(nullptr);
  EXPECT_EQ(nullptr, handle.get());

  handle.reset(nullptr);
  EXPECT_EQ(nullptr, handle.get());
  
  handle.reset(INVALID_HANDLE_VALUE);
  EXPECT_EQ(wb::base::win::kInvalidNativeHandle, handle.get());

  handle.reset(
      ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, ::GetCurrentProcessId()));
  EXPECT_NE(nullptr, handle.get());
}