// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Posix mutex.

#ifndef WB_BASE_POSIX_PTHREAD_SCOPED_MUTEX_H_
#define WB_BASE_POSIX_PTHREAD_SCOPED_MUTEX_H_

#define _OPEN_SYS_MUTEX_EXT 1
#include <pthread.h>

#include "base/macroses.h"
#include "base/deps/g3log/g3log.h"
#include "base/posix/pthread/scoped_mutex_attribute.h"
#include "base/posix/system_error_ext.h"
#include "base/std2/cstring_ext.h"

namespace wb::base::posix {

/**
 * Posix mutex.
 */
class ScopedMutex {
 public:
  /**
   * Native mutex handle type.
   */
  using native_handle_type = ::pthread_mutex_t*;

  /**
   * Create new mutex.
   * @return new mutex.
   */
  [[nodiscard]] static std2::result<ScopedMutex> New() noexcept {
    ::pthread_mutex_t mutex;
    return New(&mutex, nullptr);
  }

  /**
   * Create new mutex.
   * @param mutex_attribute Mutex attribute.
   * @return New mutex.
   */
  [[nodiscard]] static std2::result<ScopedMutex> New(
      const ScopedMutexAttribute& mutex_attribute) noexcept {
    ::pthread_mutex_t mutex;
    return New(&mutex, mutex_attribute.native_handle());
  }

  ScopedMutex(ScopedMutex&& m) noexcept : mutex_{m.mutex_} {
    m.mutex_ = empty_mutex_;
  }
  ScopedMutex& operator=(ScopedMutex&& m) noexcept {
    std::swap(mutex_, m.mutex_);
    return *this;
  }

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(ScopedMutex);

  ~ScopedMutex() noexcept {
    // If mutex is initialized (move not occurred).
    if (std2::BitwiseCompare(mutex_, empty_mutex_) != 0) {
      const std::error_code rc{
          get_error(::pthread_mutex_destroy(native_handle()))};
      G3PLOGE2_IF(WARNING, rc)
          << "Mutex " << std::hex << native_handle() << " destruction failure.";
    }
  }

  /**
   * Lock mutex.
   */
  void lock() {
    // EINVAL, EAGAIN, EBUSY, EINVAL, EDEADLK(may)
    const std::error_code rc{get_error(::pthread_mutex_lock(native_handle()))};
    if (rc) {
      G3DPLOG_E(WARNING, rc)
          << "Lock mutex " << std::hex << native_handle() << " failed.";
      throw std::system_error{rc};
    }
  }

  /**
   * Try lock mutex.
   * @return true if mutex locked, false otherwise.
   */
  [[nodiscard]] bool try_lock() noexcept {
    // XXX EINVAL, EAGAIN, EBUSY
    const std::error_code rc{get_error(pthread_mutex_trylock(native_handle()))};
    G3DPLOGE_IF(WARNING, rc ? &rc : nullptr)
        << "Try lock mutex " << std::hex << native_handle() << " failed.";
    return !rc;
  }

  /**
   * Unlock.
   */
  void unlock() {
    // XXX EINVAL, EAGAIN, EPERM
    const std::error_code rc{get_error(pthread_mutex_unlock(native_handle()))};
    G3PLOGE2_IF(WARNING, rc)
        << "Unlock mutex " << std::hex << native_handle() << " failed.";
  }

  /**
   * Mutex native handle.
   * @return Mutex native handle.
   */
  [[nodiscard]] native_handle_type native_handle() noexcept { return &mutex_; }

 private:
  /**
   * Mutex.
   */
  pthread_mutex_t mutex_;
  /**
   * Empty mutex.
   */
  static const pthread_mutex_t empty_mutex_;

  /**
   * Create scoped mutex.
   * @param mutex Native mutex.
   */
  explicit ScopedMutex(const pthread_mutex_t& mutex) noexcept : mutex_{mutex} {}

  /**
   * Create scoped mutex.
   * @param mutex Native mutex.
   * @param mutex_attribute Mutex attribute.
   * @return Scoped mutex.
   */
  [[nodiscard]] static std2::result<ScopedMutex> New(
      native_handle_type mutex,
      ScopedMutexAttribute::const_native_handle_type mutex_attribute) noexcept {
    G3DCHECK(!!mutex);
    const std::error_code rc{
        get_error(::pthread_mutex_init(mutex, mutex_attribute))};
    return !rc ? std2::result<ScopedMutex>{ScopedMutex{*mutex}}
               : std2::result<ScopedMutex>{rc};
  }
};

const pthread_mutex_t ScopedMutex::empty_mutex_ = {
    {__PTHREAD_MUTEX_INITIALIZER(-1)}};

}  // namespace wb::base::posix

#endif  // !WB_BASE_POSIX_PTHREAD_SCOPED_MUTEX_H_
