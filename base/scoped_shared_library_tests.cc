// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Wrapper for shared library.

#include "scoped_shared_library.h"
//
#ifdef WB_OS_WIN
#include "base/win/windows_light.h"
//
#include <usp10.h>
#endif
//
#include "base/deps/googletest/gtest/gtest.h"

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(ScopedSharedLibraryTest, LoadUnloadUnexistingLibraryFails) {
  using namespace wb::base;

  const auto unexisting_library_result =
      ScopedSharedLibrary::FromLibraryOnPath("some-unexisting-library", 0);

  ASSERT_FALSE(unexisting_library_result.has_value());

#ifdef WB_OS_WIN
  EXPECT_EQ(std::error_code(ERROR_MOD_NOT_FOUND, std::system_category()),
            unexisting_library_result.error());
#elif defined(WB_OS_POSIX)
  EXPECT_EQ(std::error_code(EINVAL, std::system_category()),
            unexisting_library_result.error());
#else
#error "Please add shared library support for your platform."
#endif  // !WB_OS_WIN && !WB_OS_POSIX
}

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(ScopedSharedLibraryTest, LoadUnloadLibraryInScope) {
  using namespace wb::base;

#ifdef WB_OS_WIN
  ASSERT_EQ(nullptr, ::GetModuleHandleA("Usp10.dll"));

  {
    const auto usp10_library_result = ScopedSharedLibrary::FromLibraryOnPath(
        "Usp10.dll", LOAD_LIBRARY_SEARCH_SYSTEM32);
    EXPECT_TRUE(usp10_library_result.has_value());

    const auto usp10_script_text_out_function =
        usp10_library_result->GetAddressAs<decltype(&ScriptTextOut)>(
            "ScriptTextOut");
    ASSERT_TRUE(usp10_script_text_out_function.has_value());
    EXPECT_NE(nullptr, *usp10_script_text_out_function);
  }

  ASSERT_EQ(nullptr, ::GetModuleHandleA("Usp10.dll"));
#elif defined(WB_OS_LINUX)
  {
    // Rely on dbus installed on almost every system.
    const auto dbus_library_result =
        ScopedSharedLibrary::FromLibraryOnPath("libdbus-1.so", 0);
    EXPECT_TRUE(dbus_library_result.has_value());

    struct DBusConnection;
    struct DBusError;
    using DbusBusGetIdFunction =
        char* (*)(DBusConnection* connection, DBusError* error);

    const auto dbus_bus_get_id_function =
        dbus_library_result->GetAddressAs<DbusBusGetIdFunction>(
            "dbus_bus_get_id");
    ASSERT_TRUE(dbus_bus_get_id_function.has_value());
    EXPECT_NE(nullptr, *dbus_bus_get_id_function);
  }
#else
#error "Please add shared library support test for your platform."
#endif
}