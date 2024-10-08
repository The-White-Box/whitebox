// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// DLL load utils.

#include "dll_load_utils.h"
//
#include <sal.h>

#include <filesystem>

#include "base/std2/string_ext.h"
#include "base/std2/system_error_ext.h"
//
#include "base/deps/googletest/gtest/gtest.h"

using HINSTANCE = struct HINSTANCE__*;
using HMODULE = HINSTANCE;

extern "C" WB_ATTRIBUTE_DLL_IMPORT _When_(lpModuleName == NULL, _Ret_notnull_)
    _When_(lpModuleName != NULL, _Ret_maybenull_) HMODULE
    __stdcall GetModuleHandleA(_In_opt_ const char* lpModuleName);

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(DllLoadUtilsTest, GetApplicationDirectory) {
  // NOLINTNEXTLINE(performance-no-int-to-ptr): Required for test.
  const HINSTANCE mustBeInvalidInstance{reinterpret_cast<HINSTANCE>(
      reinterpret_cast<uintptr_t>(::GetModuleHandleA(nullptr)) + 1U)};

  EXPECT_EQ(std::error_code(ERROR_MOD_NOT_FOUND, std::system_category()),
            wb::base::win::GetApplicationDirectory(mustBeInvalidInstance)
                .error_or(wb::base::std2::ok_code))
      << "Should return ERROR_MOD_NOT_FOUND system error if module not found.";

  std::error_code rc;
  const std::filesystem::path current_directory{
      std::filesystem::current_path(rc)};
  ASSERT_FALSE(rc) << "Can't get current directory: " << rc.message();

  const std::string expected_application_directory{
      current_directory.string() +
      wb::base::std2::WideToUTF8(
          std::wstring{std::filesystem::path::preferred_separator})};
  EXPECT_EQ(expected_application_directory,
            wb::base::win::GetApplicationDirectory(nullptr).value_or(std::string{}))
      << "Should get application directory with path separator in the end.";
}
