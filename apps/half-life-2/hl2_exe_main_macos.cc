// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// The entry point for Mac Half-Life 2 process.
//
// Based on
// https://github.com/chromium/chromium/blob/master/chrome/app/chrome_exe_main_mac.cc

#include <dlfcn.h>
#include <libgen.h>
#include <mach-o/dyld.h>
#include <unistd.h>

#include <cerrno>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <system_error>

#include "app_version_config.h"
#include "base/unique_module_ptr.h"
#include "bootmgr/bootmgr_main.h"

extern "C" {
// abort_report_np() records the message in a special section that both the
// system CrashReporter and Crashpad collect in crash reports. Using a Crashpad
// Annotation would be preferable, but this executable cannot depend on
// Crashpad directly.
void abort_report_np(const char* fmt, ...);
}

namespace {
[[noreturn]] void FatalError(const char* format, ...) {
  va_list valist;
  va_start(valist, format);

  char message[4096];
  if (vsnprintf(message, sizeof(message), format, valist) >= 0) {
    abort_report_np("%s", message);
  }

  va_end(valist);
  abort();
}
}  // namespace

__attribute__((visibility("default"))) int main(int argc, char* argv[]) {
  uint32_t exec_path_size{0};
  int rv{_NSGetExecutablePath(nullptr, &exec_path_size)};
  if (rv != -1) [[unlikely]] {
    FatalError("_NSGetExecutablePath: get length failed.");
  }

  std::unique_ptr<char[]> exec_path{std::make_unique<char[]>(exec_path_size)};
  rv = _NSGetExecutablePath(exec_path.get(), &exec_path_size);
  if (rv != 0) [[unlikely]] {
    FatalError("_NSGetExecutablePath: get path failed.");
  }

  constexpr char rel_path[]{
      "../Frameworks/" WB_PRODUCT_NAME_STRING
      " Framework.framework/Versions/" WB_PRODUCT_FILE_VERSION_INFO_STRING
      "/" WB_PRODUCT_FILE_DESCRIPTION_STRING " Framework"};

  // Slice off the last part of the main executable path, and append the
  // version framework information.
  const char* parent_dir{dirname(exec_path.get())};
  if (!parent_dir) [[unlikely]] {
    FatalError("dirname '%s': %s.", exec_path.get(), strerror(errno));
  }

  const size_t parent_dir_len{strlen(parent_dir)};
  const size_t rel_path_len{strlen(rel_path)};
  // 2 accounts for a trailing NUL byte and the '/' in the middle of the paths.
  const size_t framework_path_size{parent_dir_len + rel_path_len + 2};
  std::unique_ptr<char[]> framework_path{
      std::make_unique<char[]>(framework_path_size)};
  snprintf(framework_path.get(), framework_path_size, "%s/%s", parent_dir,
           rel_path);

  const auto bootmgr_load_result =
      wb::base::unique_module_ptr::FromLibraryOnPath(
          framework_path.get(), RTLD_LAZY | RTLD_LOCAL | RTLD_FIRST);
  if (const auto* rc = std::get_if<std::error_code>(&bootmgr_load_result))
      [[unlikely]] {
    FatalError("Can't load boot manager '%s': %s.", framework_path.get(),
               rc->message());
  }

  const auto bootmgr_module =
      std::get<wb::base::unique_module_ptr>(bootmgr_load_result);
  constexpr char kBootmgrMainFunctionName[]{"BootmgrMain"};

  using BootmgrMainFunction = decltype(&BootmgrMain);

  // Good, try to find and launch boot manager.
  const auto bootmgr_entry_result =
      bootmgr_module->GetAddressAs<BootmgrMainFunction>(
          kBootmgrMainFunctionName);
  if (const auto* rc = std::get_if<std::error_code>(&bootmgr_entry_result))
      [[unlikely]] {
    FatalError("Can't get '%s' entry point from '%s': %s.",
               kBootmgrMainFunctionName, framework_path.get(), rc->message());
  }

  const auto bootmgr_main = std::get<BootmgrMainFunction>(bootmgr_entry_result);

  rv = bootmgr_main(argc, argv);

  // exit, don't return from main, to avoid the apparent removal of main
  // from stack backtraces under tail call optimization.
  exit(rv);
}