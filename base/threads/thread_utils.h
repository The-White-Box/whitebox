// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Thread extensions.

#ifndef WB_BASE_THREADS_THREAD_UTILS_H_
#define WB_BASE_THREADS_THREAD_UTILS_H_

#include <string>
#include <thread>

#include "base/base_api.h"
#include "base/base_macroses.h"
#include "base/deps/g3log/g3log.h"
#include "base/std2/system_error_ext.h"
#include "build/compiler_config.h"

#ifdef WB_OS_WIN
#include <sal.h>
#endif

namespace wb::base::threads {

/**
 * @brief Native thread handle.
 */
using NativeThreadHandle = std::thread::native_handle_type;

#ifdef WB_OS_WIN
using NativeThreadName = std::wstring;
#else
using NativeThreadName = std::string;
#endif

/**
 * Gets current thread handle.
 * @return Native thread handle.
 */
[[nodiscard]] WB_ATTRIBUTE_CONST WB_BASE_API NativeThreadHandle
GetCurrentThreadHandle() noexcept;

/**
 * @brief Gets current thread name.
 * @param handle Thread handle.
 * @param thread_name Thread name.
 * @return Error code.
 */
[[nodiscard]] WB_BASE_API std::error_code GetThreadName(
    NativeThreadHandle handle, NativeThreadName &thread_name) noexcept;

/**
 * @brief Set current thread name.
 * @param thread_name New thread name.
 * @return Error code.
 */
[[nodiscard]] WB_BASE_API std::error_code SetThreadName(
    const NativeThreadName &thread_name) noexcept;

/**
 * @brief Scoped thread name.
 */
class ScopedThreadName {
 public:
  /**
   * @brief Set name for current thread in scope and restore out of scope.
   * @param new_thread_name Scoped thread name.
   * @return ScopedThreadName.
   */
  [[nodiscard]] static std2::result<ScopedThreadName> New(
      const NativeThreadName &new_thread_name) {
    ScopedThreadName name{GetCurrentThreadHandle(), new_thread_name};
    return !name.error_code()
               ? std2::result<ScopedThreadName>{std::move(name)}
               : std2::result<ScopedThreadName>{name.error_code()};
  }

  ScopedThreadName(ScopedThreadName &&n) noexcept
      : thread_{n.thread_},
        old_thread_name_{std::move(n.old_thread_name_)},
        error_code_{n.error_code_} {
    n.error_code_ = std::error_code{EINVAL, std::system_category()};

    G3DCHECK(thread_ == GetCurrentThreadHandle())
        << "Thread name should be moved for original thread.";
  }
  ScopedThreadName &operator=(ScopedThreadName &&) noexcept = delete;

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(ScopedThreadName);

  /**
   * @brief Restore previous thread name.
   */
  ~ScopedThreadName() noexcept {
    G3DCHECK(thread_ == GetCurrentThreadHandle())
        << "Thread name should be restored for original thread.";

    if (!error_code()) {
      G3CHECK(!SetThreadName(old_thread_name_));
    }
  }

  /**
   * @brief Get error code.
   * @return Error code.
   */
  [[nodiscard]] std::error_code error_code() const noexcept {
    return error_code_;
  }

 private:
  /**
   * @brief Thread handle.
   */
  NativeThreadHandle thread_;
  /**
   * @brief Previous thread name.
   */
  NativeThreadName old_thread_name_;
  /**
   * @brief Error code.
   */
  std::error_code error_code_;

  /**
   * @brief Set name for thread in scope and restore out of scope.
   * @param thread Thread.
   * @param new_thread_name Scoped thread name.
   */
  explicit ScopedThreadName(NativeThreadHandle thread,
                            const NativeThreadName &new_thread_name)
      : thread_{thread},
        old_thread_name_{},
        error_code_{GetThreadName(thread, old_thread_name_)} {
    G3CHECK(!error_code());

    if (!error_code()) error_code_ = SetThreadName(new_thread_name);
  }
};

}  // namespace wb::base::threads

#endif  // !WB_BASE_THREADS_THREAD_UTILS_H_
