// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Thread extensions.

#include "thread_utils.h"

#include <string>
#include <string_view>

#include "base/base_macroses.h"
#include "base/deps/g3log/g3log.h"
#include "base/std_ext/cstring_ext.h"
#include "build/build_config.h"

#ifdef WB_OS_WIN
#include <sal.h>

#include "base/windows/memory/scoped_local_memory.h"
#include "base/windows/system_error_ext.h"

using HANDLE = void*;
using HRESULT = long;

extern "C" WB_ATTRIBUTE_DLL_IMPORT HRESULT __stdcall GetThreadDescription(
    _In_ HANDLE thread, _Outptr_result_z_ wchar_t** description);
extern "C" WB_ATTRIBUTE_DLL_IMPORT HRESULT __stdcall SetThreadDescription(
    _In_ HANDLE thread, _In_ const wchar_t* description);
#elif defined(WB_OS_POSIX)
#include <pthread.h>
#endif

namespace wb::base::threads {
/**
 * @brief Gets thread name.
 * @param handle Thread handle.
 * @param thread_name Thread name.
 * @return Error code.
 */
[[nodiscard]] WB_BASE_API std::error_code GetThreadName(
    _In_ ThreadHandle handle, _Out_ std::string& thread_name) {
#ifdef WB_OS_WIN
  // Minimum supported client is Windows 10, version 1607.
  wchar_t* wide_thread_name;

  const std::error_code rc{
      windows::GetErrorCode(::GetThreadDescription(handle, &wide_thread_name))};
  const windows::memory::ScopedLocalMemory scoped_local_memory{
      wide_thread_name};

  if (!rc) {
    thread_name =
        std_ext::WideToAnsi({wide_thread_name, ::wcslen(wide_thread_name)});
  }

  return rc;
#elif defined(WB_OS_POSIX)
  thread_name.resize(32);

  return GetThreadPosixErrorCode(
      ::pthread_getname_np(handle, thread_name.data(), thread_name.size()));
#else
#error Please, define GetThreadName for your os in base/include/threads/thread_utils.cc
#endif
}

/**
 * @brief Set thread name.
 * @param handle Thread handle.
 * @param thread_name New thread name.
 * @return Error code.
 */
[[nodiscard]] WB_BASE_API std::error_code SetThreadName(
    _In_ ThreadHandle handle, _In_ std::string_view thread_name) {
#ifdef WB_OS_WIN
  // Minimum supported client is Windows 10, version 1607.
  const std::wstring wide_thread_name{std_ext::AnsiToWide(thread_name)};

  return windows::GetErrorCode(
      ::SetThreadDescription(handle, wide_thread_name.c_str()));
#elif defined(WB_OS_POSIX)
  return GetThreadPosixErrorCode(
      ::pthread_setname_np(handle, thread_name.data()));
#else
#error Please, define SetThreadName for your os in base/include/threads/thread_utils.cc
#endif
}
}  // namespace wb::base::threads
