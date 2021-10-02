// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Thread extensions.

#include "thread_utils.h"

#include "base/deps/g3log/g3log.h"
#include "build/build_config.h"

#ifdef WB_OS_WIN
#include <sal.h>

#include "base/win/memory/scoped_local_memory.h"
#include "base/win/system_error_ext.h"

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
 * Gets current thread handle.
 * @return Native thread handle.
 */
[[nodiscard]] WB_ATTRIBUTE_CONST WB_BASE_API NativeThreadHandle
GetCurrentThreadHandle() noexcept {
#ifdef WB_OS_WIN
  return NativeThreadHandle{::GetCurrentThread()};
#elif defined(WB_OS_POSIX)
  return NativeThreadHandle{pthread_self()};
#else
#error Please define GetCurrentThreadHandle for your platform.
#endif
}

/**
 * @brief Gets thread name.
 * @param handle Thread handle.
 * @param thread_name Thread name.
 * @return Error code.
 */
[[nodiscard]] WB_BASE_API std::error_code GetThreadName(
    NativeThreadHandle handle, NativeThreadName& thread_name) noexcept {
#ifdef WB_OS_WIN
  // Minimum supported client is Windows 10, version 1607.
  wchar_t* wide_thread_name;

  const std::error_code rc{
      windows::get_error(::GetThreadDescription(handle, &wide_thread_name))};
  const windows::memory::ScopedLocalMemory scoped_local_memory{
      wide_thread_name};

  if (!rc) WB_ATTRIBUTE_LIKELY {
      thread_name = wide_thread_name;
    }
  else {
    thread_name = L"";
  }

  return rc;
#elif defined(WB_OS_POSIX)
  thread_name.resize(32);  //-V112

  return std2::posix_last_error_code(
      ::pthread_getname_np(handle, thread_name.data(), thread_name.size()));
#else
#error Please, define GetThreadName for your platform.
#endif
}

/**
 * @brief Set thread name.
 * @param thread_name New thread name.
 * @return Error code.
 */
[[nodiscard]] WB_BASE_API std::error_code SetThreadName(
    const NativeThreadName& thread_name) noexcept {
#ifdef WB_OS_WIN
  return windows::get_error(
      ::SetThreadDescription(GetCurrentThreadHandle(), thread_name.c_str()));
#elif defined(WB_OS_POSIX)
#ifdef WB_OS_MACOS
  return std2::posix_last_error_code(::pthread_setname_np(thread_name.c_str()));
#else
  return std2::posix_last_error_code(
      ::pthread_setname_np(GetCurrentThreadHandle(), thread_name.c_str()));
#endif
#else
#error Please, define SetThreadName for your platform.
#endif
}

}  // namespace wb::base::threads
