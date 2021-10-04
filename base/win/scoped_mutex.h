// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.

#ifndef WB_BASE_WIN_SCOPED_MUTEX_H_
#define WB_BASE_WIN_SCOPED_MUTEX_H_

#include <chrono>

#include "base/base_macroses.h"
#include "base/deps/g3log/g3log.h"
#include "base/std2/system_error_ext.h"
#include "base/win/security/os_objects_access_rights.h"
#include "base/win/unique_handle.h"

using HANDLE = void *;
using SECURITY_ATTRIBUTES = struct _SECURITY_ATTRIBUTES;

extern "C" WB_ATTRIBUTE_DLL_IMPORT _Ret_maybenull_
    HANDLE __stdcall CreateMutexExA(
        _In_opt_ SECURITY_ATTRIBUTES *lpMutexAttributes,
        _In_opt_ const char *lpName, _In_ unsigned long dwFlags,
        _In_ unsigned long dwDesiredAccess);

extern "C" WB_ATTRIBUTE_DLL_IMPORT unsigned long __stdcall WaitForSingleObject(
    _In_ HANDLE hHandle, _In_ unsigned long dwMilliseconds);

namespace wb::base::windows {

/**
 * @brief Mutex creation flags.
 */
enum class ScopedMutexCreationFlag : unsigned long {
  /**
   * @brief No flags.
   */
  kNone = 0UL,
  /**
   * @brief The object creator is the initial owner of the mutex.
   */
  kMakeMeOwner = 0x0000'0001UL
};

/**
 * @brief Mutex wait result.
 */
enum class ScopedMutexWaitStatus : unsigned long {
  /**
   * @brief The state of the specified object is signaled.
   */
  kSignalled = 0x0000'0000UL,
  /**
   * @brief The specified object is a mutex object that was not released by the
   * thread that owned the mutex object before the owning thread terminated.
   * Ownership of the mutex object is granted to the calling thread and the
   * mutex state is set to nonsignaled.  If the mutex was protecting persistent
   * state information, you should check it for consistency.
   */
  kAbandoned = 0x0000'0080UL,
  /**
   * @brief The time - out interval elapsed, and the object's state is
   * nonsignaled.
   */
  kTimeout = 0x0000'0102UL,
  /**
   * @brief The function has failed.  To get extended error information, call
   * std2::system_last_error_code.
   */
  kFailed = 0xFFFF'FFFFUL
};

/**
 * @brief Scoped OS mutex.
 */
class ScopedMutex {
 public:
  /**
   * @brief Create scoped OS mutex.
   * @param security_attributes Security attributes for mutex.
   * @param name Mutex name.
   * @param flags Mutex flags.
   * @param access_rights Mutex access rights.
   * @return nothing.
   */
  [[nodiscard]] static std2::result<ScopedMutex> New(
      _In_opt_ SECURITY_ATTRIBUTES *security_attributes,
      _In_opt_ const char *name, _In_ ScopedMutexCreationFlag flags,
      _In_ const security::ScopedMutexAccessRights &access_rights) noexcept {
    ScopedMutex mutex{security_attributes, name, flags, access_rights};
    return !mutex.error_code() ? std2::result<ScopedMutex>{std::move(mutex)}
                               : std2::result<ScopedMutex>{mutex.error_code()};
  }

  ScopedMutex(ScopedMutex &&s) noexcept
      : mutex_{std::move(s.mutex_)}, error_code_{s.error_code_} {
    s.error_code_ = std::error_code{EINVAL, std::system_category()};
  }
  ScopedMutex &operator=(ScopedMutex &&) = delete;

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(ScopedMutex);

  /**
   * @brief Wait for mutex changes state.
   * @tparam TRep
   * @tparam TPeriod
   * @param timeout Timeout to wait, or exit with
   * ScopedMutexWaitStatus::kTimeout.
   * @return Wait status.
   */
  template <class TRep, class TPeriod>
  ScopedMutexWaitStatus WaitForSingle(
      const std::chrono::duration<TRep, TPeriod> &timeout) const noexcept {
    G3DCHECK(!!mutex_.get());

    const auto timeout_milliseconds =
        std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count();

    G3DCHECK(timeout_milliseconds >= 0 &&
             timeout_milliseconds <= std::numeric_limits<unsigned long>::max());

    return static_cast<ScopedMutexWaitStatus>(::WaitForSingleObject(
        mutex_.get(), static_cast<unsigned long>(timeout_milliseconds)));
  }

 private:
  /**
   * @brief Mutex handle.
   */
  unique_handle mutex_;
  /**
   * @brief Mutex creation error code.
   */
  std::error_code error_code_;

  /**
   * @brief Create scoped mutex.
   * @param security_attributes Mutex security attributes.
   * @param name Mutex name.
   * @param flags Mutex flags.
   * @param access_rights Mutex access rights.
   * @return nothing.
   */
  ScopedMutex(
      _In_opt_ SECURITY_ATTRIBUTES *security_attributes,
      _In_opt_ const char *name, _In_ ScopedMutexCreationFlag flags,
      _In_ const security::ScopedMutexAccessRights &access_rights) noexcept
      : mutex_{reinterpret_cast<handle_descriptor *>(
            ::CreateMutexExA(security_attributes, name, underlying_cast(flags),
                             access_rights.Value()))},
        // If the mutex is a named mutex and the object existed before this
        // function call, the return value is a handle to the existing object,
        // and the GetLastError function returns ERROR_ALREADY_EXISTS.
        //
        // Need to catch case above here.  Assume such mutex can't be created.
        error_code_{std2::system_last_error_code()} {}

  /**
   * @brief Get error code.
   * @return Error code.
   */
  [[nodiscard]] std::error_code error_code() const noexcept {
    return error_code_;
  }
};

}  // namespace wb::base::windows

#endif  // !WB_BASE_WIN_SCOPED_MUTEX_H_
