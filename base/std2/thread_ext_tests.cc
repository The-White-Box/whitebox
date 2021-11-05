// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// <thread> extensions.

#include "thread_ext.h"
//
#ifdef WB_OS_WIN
#include "base/win/windows_light.h"
#endif

#include "base/deps/googletest/gtest/gtest.h"

using namespace wb::base;

// NOLINTNEXTLINE(cert-err58-cpp, cppcoreguidelines-avoid-non-const-global-variables, cppcoreguidelines-owning-memory)
GTEST_TEST(ThreadExtTests, this_thread_get_handle) {
  const std2::native_thread_handle actual_thread_handle{
      std2::this_thread::get_handle()};

#ifdef WB_OS_WIN
  const std2::native_thread_handle expected_thread_handle{::GetCurrentThread()};
#elif defined(WB_OS_POSIX)
  const std2::native_thread_handle expected_thread_handle{pthread_self()};
#else
#error "Please define get_handle for your platform."
#endif

  EXPECT_EQ(actual_thread_handle, expected_thread_handle);
}

// NOLINTNEXTLINE(cert-err58-cpp, cppcoreguidelines-avoid-non-const-global-variables, cppcoreguidelines-owning-memory)
GTEST_TEST(ThreadExtTests, get_thread_name) {
  std2::native_thread_name thread_name{"get_thread_name"};

  EXPECT_EQ(std2::ok_code, std2::this_thread::set_name(thread_name));
  EXPECT_EQ(std2::ok_code,
            std2::get_thread_name(std2::this_thread::get_handle(), thread_name));

  EXPECT_EQ(thread_name, std2::native_thread_name{"get_thread_name"});
}

// NOLINTNEXTLINE(cert-err58-cpp, cppcoreguidelines-avoid-non-const-global-variables, cppcoreguidelines-owning-memory)
GTEST_TEST(ThreadExtTests, this_thread_set_name) {
#ifdef WB_OS_WIN
  const std2::native_thread_name expected_thread_name{L"this_thread_set_name"};
#elif defined(WB_OS_POSIX)
  const std2::native_thread_name expected_thread_name{"small_name"};
#else
#error "Please define native_thread_name for your platform."
#endif

  EXPECT_EQ(std2::ok_code, std2::this_thread::set_name(expected_thread_name));

  std2::native_thread_name actual_thread_name;
  EXPECT_EQ(std2::ok_code,
            std2::get_thread_name(std2::this_thread::get_handle(),
                                  actual_thread_name));

  EXPECT_EQ(actual_thread_name, expected_thread_name);
}

// NOLINTNEXTLINE(cert-err58-cpp, cppcoreguidelines-avoid-non-const-global-variables, cppcoreguidelines-owning-memory)
GTEST_TEST(ThreadExtTests, this_thread_ScopedThreadNameScope) {
#ifdef WB_OS_WIN
  const std2::native_thread_name expected_thread_name{
      L"this_thread_ScopedThreadName"};
#elif defined(WB_OS_POSIX)
  const std2::native_thread_name expected_thread_name{
      "ScopedName"};
#else
#error "Please define native_thread_name for your platform."
#endif

  std2::native_thread_name original_thread_name;
  EXPECT_EQ(std2::ok_code,
            std2::get_thread_name(std2::this_thread::get_handle(),
                                  original_thread_name));
  EXPECT_NE(original_thread_name, expected_thread_name);

  {
    auto scoped_thread_name =
        std2::this_thread::ScopedThreadName::New(expected_thread_name);
    EXPECT_EQ(nullptr, std2::get_error(scoped_thread_name));

    std2::native_thread_name new_thread_name;
    EXPECT_EQ(std2::ok_code,
              std2::get_thread_name(std2::this_thread::get_handle(),
                                    new_thread_name));
    EXPECT_EQ(new_thread_name, expected_thread_name);
  }
  
  EXPECT_EQ(std2::ok_code,
            std2::get_thread_name(std2::this_thread::get_handle(),
                                  original_thread_name));  
  EXPECT_NE(original_thread_name, expected_thread_name);
}

// NOLINTNEXTLINE(cert-err58-cpp, cppcoreguidelines-avoid-non-const-global-variables, cppcoreguidelines-owning-memory)
GTEST_TEST(ThreadExtTests, this_thread_ScopedThreadNameMoveConstructor) {
#ifdef WB_OS_WIN
  const std2::native_thread_name expected_thread_name{
      L"this_thread_ScopedThreadName"};
#elif defined(WB_OS_POSIX)
  const std2::native_thread_name expected_thread_name{
      "ScopedName"};
#else
#error "Please define native_thread_name for your platform."
#endif

  std2::native_thread_name original_thread_name;
  EXPECT_EQ(std2::ok_code,
            std2::get_thread_name(std2::this_thread::get_handle(),
                                  original_thread_name));
  EXPECT_NE(original_thread_name, expected_thread_name);

  {
    auto scoped_thread_name_result =
        std2::this_thread::ScopedThreadName::New(expected_thread_name);
    auto *scoped_thread_name = std2::get_result(scoped_thread_name_result);
    ASSERT_NE(nullptr, scoped_thread_name);

    auto moved_thread_name = std::move(*scoped_thread_name);

    std2::native_thread_name new_thread_name;
    EXPECT_EQ(std2::ok_code,
              std2::get_thread_name(std2::this_thread::get_handle(),
                                    new_thread_name));
    EXPECT_EQ(new_thread_name, expected_thread_name);
  }
  
  EXPECT_EQ(std2::ok_code,
            std2::get_thread_name(std2::this_thread::get_handle(),
                                  original_thread_name));  
  EXPECT_NE(original_thread_name, expected_thread_name);
}
