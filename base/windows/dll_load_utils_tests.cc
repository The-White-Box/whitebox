// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// DLL load utils.

#include "dll_load_utils.h"
//
#include <sal.h>

#include <filesystem>

#include "base/std_ext/cstring_ext.h"
//
#include "base/deps/googletest/gtest/gtest.h"

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(DllLoadUtilsTest, MustBeSignedDllLoadTarget) {
  EXPECT_TRUE(wb::base::windows::MustBeSignedDllLoadTarget(""));
  EXPECT_TRUE(wb::base::windows::MustBeSignedDllLoadTarget("program_name"));
  EXPECT_TRUE(wb::base::windows::MustBeSignedDllLoadTarget(
      "program_name --type network-driver"));
  EXPECT_TRUE(wb::base::windows::MustBeSignedDllLoadTarget(
      "--prepend--insecure-allow-unsigned-module-target"));
  EXPECT_TRUE(wb::base::windows::MustBeSignedDllLoadTarget(
      "--insecure-allow-unsigned-module-target--append"));

  EXPECT_FALSE(wb::base::windows::MustBeSignedDllLoadTarget(
      "--previous-flag --insecure-allow-unsigned-module-target"));
  EXPECT_FALSE(wb::base::windows::MustBeSignedDllLoadTarget(
      "--previous-flag-tab\t--insecure-allow-unsigned-module-target"));

  EXPECT_FALSE(wb::base::windows::MustBeSignedDllLoadTarget(
      "--insecure-allow-unsigned-module-target"));
  EXPECT_FALSE(wb::base::windows::MustBeSignedDllLoadTarget(
      " --insecure-allow-unsigned-module-target "));
  EXPECT_FALSE(wb::base::windows::MustBeSignedDllLoadTarget(
      "\t--insecure-allow-unsigned-module-target\t"));

  EXPECT_FALSE(wb::base::windows::MustBeSignedDllLoadTarget(
      "--insecure-allow-unsigned-module-target --next-flag"));
  EXPECT_FALSE(wb::base::windows::MustBeSignedDllLoadTarget(
      "--insecure-allow-unsigned-module-target\t--next-flag"));
}

using HINSTANCE = struct HINSTANCE__*;
using HMODULE = HINSTANCE;

extern "C" WB_ATTRIBUTE_DLL_IMPORT _When_(lpModuleName == NULL, _Ret_notnull_)
    _When_(lpModuleName != NULL, _Ret_maybenull_) HMODULE
    __stdcall GetModuleHandleA(_In_opt_ const char* lpModuleName);

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(DllLoadUtilsTest, GetApplicationDirectory) {
  const HINSTANCE mustBeInvalidInstance{reinterpret_cast<HINSTANCE>(
      reinterpret_cast<uintptr_t>(::GetModuleHandleA(nullptr)) + 1U)};

  EXPECT_EQ(
      std::error_code(ERROR_MOD_NOT_FOUND, std::system_category()),
      std::get<std::error_code>(
          wb::base::windows::GetApplicationDirectory(mustBeInvalidInstance)))
      << "Should return ERROR_MOD_NOT_FOUND system error if module not found.";

  std::error_code rc;
  const std::filesystem::path current_directory{
      std::filesystem::current_path(rc)};
  ASSERT_FALSE(rc) << "Can't get current directory: " << rc.message();

  const std::string expected_application_directory{
      current_directory.string() +
      wb::base::std_ext::WideToUTF8(
          std::wstring{std::filesystem::path::preferred_separator})};
  EXPECT_EQ(expected_application_directory,
            std::get<std::string>(
                wb::base::windows::GetApplicationDirectory(nullptr)))
      << "Should get application directory with path separator in the end.";
}
