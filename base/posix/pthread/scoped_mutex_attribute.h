// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// A mutex attribute object allows you to manage the characteristics of mutexes
// in your application.  It defines the set of values to be used for a mutex
// during its creation.  By establishing a mutex attribute object, you can
// create many mutexes with the same set of characteristics, without needing to
// define the characteristics for each and every mutex.

#ifndef WB_BASE_POSIX_PTHREAD_SCOPED_MUTEX_ATTRIBUTE_H_
#define WB_BASE_POSIX_PTHREAD_SCOPED_MUTEX_ATTRIBUTE_H_

#define _OPEN_SYS_MUTEX_EXT 1
#include <pthread.h>

#include "base/macroses.h"
#include "base/deps/g3log/g3log.h"
#include "base/posix/system_error_ext.h"
#include "base/std2/cstring_ext.h"

namespace wb::base::posix {

/**
 * Attribute pshared for the mutex attribute object.
 */
enum class ScopedMutexProcessAttribute : int {
  /**
   * A mutex can only be operated upon by threads created within the same
   * process as the thread that initialized the mutex; if threads of differing
   * processes attempt to operate on such a mutex, only the process to
   * initialize the mutex will succeed.  When a new process is created by the
   * parent process it will receive a different copy of the private mutex and
   * this new mutex can only be used to serialize between threads in the child
   * process.  This is the default value of the attribute.
   */
  kPrivate = PTHREAD_PROCESS_PRIVATE,
  /**
   * Permits a mutex to be operated upon by any thread that has access to the
   * memory where the mutex is allocated, even if the mutex is allocated in
   * memory that is shared by multiple processes.
   */
  kShared = PTHREAD_PROCESS_SHARED
};

/**
 * A mutex attribute object allows you to manage the characteristics of mutexes
 * in your application.
 */
class ScopedMutexAttribute {
 public:
  /**
   * Native mutex attribute handle type.
   */
  using native_handle_type = ::pthread_mutexattr_t*;
  /**
   * Native mutex attribute handle type.
   */
  using const_native_handle_type = const ::pthread_mutexattr_t*;

  /**
   * Create mutex attribute.
   * @return Mutex attribute.
   */
  [[nodiscard]] static std2::result<ScopedMutexAttribute> New() noexcept {
    pthread_mutexattr_t attribute;
    const std::error_code rc{get_error(::pthread_mutexattr_init(&attribute))};
    return !rc ? std2::result<ScopedMutexAttribute>{ScopedMutexAttribute{
                     attribute}}
               : std2::result<ScopedMutexAttribute>{rc};
  }

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(ScopedMutexAttribute);

  ScopedMutexAttribute(ScopedMutexAttribute&& m) noexcept
      : attribute_{m.attribute_} {
    m.attribute_ = empty_attribute_;
  }
  ScopedMutexAttribute& operator=(ScopedMutexAttribute&& m) noexcept {
    std::swap(attribute_, m.attribute_);
    return *this;
  }

  ~ScopedMutexAttribute() noexcept {
    // If mutex attribute is initialized (move not occurred).
    if (std2::BitwiseCompare(attribute_, empty_attribute_) != 0) {
      const std::error_code rc{
          get_error(::pthread_mutexattr_destroy(native_handle()))};
      G3PLOGE2_IF(WARNING, rc)
          << "Mutex attribute " << std::hex << native_handle()
          << " destruction failure.";
    }
  }

  /**
   * Get native mutex attribute handle.
   * @return Native mutex attribute handle.
   */
  [[nodiscard]] native_handle_type native_handle() noexcept {
    return &attribute_;
  }

  /**
   * Get native mutex attribute handle.
   * @return Native mutex attribute handle.
   */
  [[nodiscard]] const_native_handle_type native_handle() const noexcept {
    return &attribute_;
  }

  /**
   * Set pshared mutex attribute flag.
   * @param process_shared pshared mutex attribute flag.
   * @return std::error_code - either flag is set or not.
   */
  [[nodiscard]] std::error_code set_process_shared(
      ScopedMutexProcessAttribute process_shared) noexcept {
    const std::error_code rc{get_error(::pthread_mutexattr_setpshared(
        native_handle(), underlying_cast(process_shared)))};
    G3DCHECK(!rc) << "Unable to set process shared attribute.";
    return rc;
  }

 private:
  /**
   * Mutex attribute.
   */
  pthread_mutexattr_t attribute_;
  /**
   * Empty aka none mutex attribute.
   */
  static const pthread_mutexattr_t empty_attribute_;

  /**
   * Construct mutex attribute by native mutex attribute.
   * @param attribute Native mutex attribute.
   */
  explicit ScopedMutexAttribute(const pthread_mutexattr_t& attribute) noexcept
      : attribute_{attribute} {}
};

const pthread_mutexattr_t ScopedMutexAttribute::empty_attribute_ = {{-1}};

}  // namespace wb::base::posix

#endif  // !WB_BASE_POSIX_PTHREAD_SCOPED_MUTEX_ATTRIBUTE_H_
