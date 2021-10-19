// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// <thread> extensions.

#include "thread_ext.h"

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

namespace wb::base::std2 {

/**
 * @brief Gets thread name.
 * @param handle Thread handle.
 * @param thread_name Thread name.
 * @return Error code.
 */
[[nodiscard]] WB_BASE_API std::error_code get_thread_name(
    native_thread_handle handle, native_thread_name& thread_name) noexcept {
#ifdef WB_OS_WIN
  // Minimum supported client is Windows 10, version 1607.
  wchar_t* wide_thread_name;

  const std::error_code rc{
      win::get_error(::GetThreadDescription(handle, &wide_thread_name))};
  const win::memory::ScopedLocalMemory scoped_local_memory{wide_thread_name};

  if (!rc) WB_ATTRIBUTE_LIKELY {
      thread_name = wide_thread_name;
    }
  else {
    thread_name = L"";
  }

  return rc;
#elif defined(WB_OS_POSIX)
  constexpr size_t kEstimateThreadNameSize{32};  //-V112
  thread_name.resize(kEstimateThreadNameSize);

  return std2::posix_last_error_code(
      ::pthread_getname_np(handle, thread_name.data(), thread_name.size()));
#else
#error Please, define get_thread_name for your platform.
#endif
}

namespace this_thread {

/**
 * Gets current thread handle.
 * @return Native thread handle.
 */
[[nodiscard]] WB_BASE_API native_thread_handle get_handle() noexcept {
#ifdef WB_OS_WIN
  return native_thread_handle{::GetCurrentThread()};
#elif defined(WB_OS_POSIX)
  return native_thread_handle{pthread_self()};
#else
#error Please define get_handle for your platform.
#endif
}

/**
 * @brief Set thread name.
 * @param thread_name New thread name.
 * @return Error code.
 */
[[nodiscard]] WB_BASE_API std::error_code set_name(
    const native_thread_name& thread_name) noexcept {
#ifdef WB_OS_WIN
  return win::get_error(
      ::SetThreadDescription(get_handle(), thread_name.c_str()));
#elif defined(WB_OS_POSIX)
#ifdef WB_OS_MACOS
  return std2::posix_last_error_code(::pthread_setname_np(thread_name.c_str()));
#else
  return std2::posix_last_error_code(
      ::pthread_setname_np(get_handle(), thread_name.c_str()));
#endif
#else
#error Please, define set_name for your platform.
#endif
}

}  // namespace this_thread

}  // namespace wb::base::std2
