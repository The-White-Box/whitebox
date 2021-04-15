// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// <thread> extensions.

#ifndef WB_BASE_STD2_THREAD_EXT_H_
#define WB_BASE_STD2_THREAD_EXT_H_

#include <string>
#include <thread>

#include "base/config.h"
#include "base/macroses.h"
#include "base/deps/g3log/g3log.h"
#include "base/std2/system_error_ext.h"
#include "build/compiler_config.h"

#ifdef WB_OS_WIN
#include <sal.h>
#endif

namespace wb::base::std2 {

/**
 * @brief Native thread handle.
 */
using native_thread_handle = std::thread::native_handle_type;

/**
 * @brief Native thread name.
 */
using native_thread_name = std::string;

/**
 * @brief Gets current thread name.
 * @param handle Thread handle.
 * @param thread_name Thread name.
 * @return Error code.
 */
[[nodiscard]] WB_BASE_API std::error_code get_thread_name(
    native_thread_handle handle, native_thread_name &thread_name) noexcept;

namespace this_thread {

WB_GCC_BEGIN_WARNING_OVERRIDE_SCOPE()
  // Can't use const attribute here, as thread can change under the hood,
  // so observable state of the program can change between calls (ex. in new
  // thread scope).
  WB_GCC_DISABLE_SUGGEST_CONST_ATTRIBUTE_WARNING()
  /**
   * Gets current thread handle.
   * @return Native thread handle.
   */
  [[nodiscard]] WB_BASE_API native_thread_handle get_handle() noexcept;
WB_GCC_END_WARNING_OVERRIDE_SCOPE()

/**
 * @brief Set current thread name.
 * @param thread_name New thread name.  Should be 15 symbols or less for Linux
 * distros and 63 or less for Mac.
 * @return Error code.
 */
[[nodiscard]] WB_BASE_API std::error_code set_name(
    const native_thread_name &thread_name) noexcept;

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
      const native_thread_name &new_thread_name) {
    ScopedThreadName name{get_handle(), new_thread_name};
    return !name.error_code()
               ? std2::result<ScopedThreadName>{std::move(name)}
               : std2::result<ScopedThreadName>{name.error_code()};
  }

  ScopedThreadName(ScopedThreadName &&n) noexcept
      : thread_{n.thread_},
        old_thread_name_{std::move(n.old_thread_name_)},
        error_code_{n.error_code_} {
    n.error_code_ = std2::posix_last_error_code(EINVAL);

    G3DCHECK(thread_ == get_handle())
        << "Thread name should be moved for original thread.";
  }
  ScopedThreadName &operator=(ScopedThreadName &&) noexcept = delete;

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(ScopedThreadName);

  /**
   * @brief Restore previous thread name.
   */
  ~ScopedThreadName() noexcept {
    G3DCHECK(thread_ == get_handle())
        << "Thread name should be restored for original thread.";

    if (!error_code()) {
      G3CHECK(!set_name(old_thread_name_));
    }
  }

 private:
  /**
   * @brief Thread handle.
   */
  native_thread_handle thread_;
  /**
   * @brief Previous thread name.
   */
  native_thread_name old_thread_name_;
  /**
   * @brief Error code.
   */
  std::error_code error_code_;

  /**
   * @brief Set name for thread in scope and restore out of scope.
   * @param thread Thread.
   * @param new_thread_name Scoped thread name.
   */
  explicit ScopedThreadName(native_thread_handle thread,
                            const native_thread_name &new_thread_name)
      : thread_{thread},
        old_thread_name_{},
        error_code_{get_thread_name(thread, old_thread_name_)} {
    G3CHECK(!error_code());

    if (!error_code()) error_code_ = set_name(new_thread_name);
  }

  /**
   * @brief Get error code.
   * @return Error code.
   */
  [[nodiscard]] std::error_code error_code() const noexcept {
    return error_code_;
  }
};

}  // namespace this_thread

}  // namespace wb::base::std2

#endif  // !WB_BASE_STD2_THREAD_EXT_H_
