// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Ensures single app instance is running.

#ifndef WB_WHITEBOX_BOOT_MANAGER_SCOPED_APP_INSTANCE_MANAGER_H_
#define WB_WHITEBOX_BOOT_MANAGER_SCOPED_APP_INSTANCE_MANAGER_H_

#include <array>
#include <string>

#include "base/macroses.h"
#include "base/std2/system_error_ext.h"
#include "build/build_config.h"

#ifdef WB_OS_WIN
#include "base/win/scoped_mutex.h"
#elif defined(WB_OS_POSIX)
#include "base/posix/scoped_shared_memory_object.h"
#endif

namespace wb::boot_manager {

/**
 * @brief App instance status.
 */
enum class AppInstanceStatus {
  /**
   * @brief Not other app instances.
   */
  kNoOtherInstances,
  /**
   * @brief Has at least one other instance of the app.
   */
  kAlreadyRunning,
  /**
   * @brief Failed to determine either it is single app instance or not.
   */
  kUnableToDetermine
};

/**
 * @brief Manages application instances.
 */
class ScopedAppInstanceManager {
 public:
  /**
   * @brief Create app instance manager.
   * @param app_description Application description.
   * @return nothing.
   */
  explicit ScopedAppInstanceManager(const char* app_description) noexcept
#ifdef WB_OS_WIN
      : app_instance_mutex_{base::win::ScopedMutex::New(
            nullptr, MakeMutexName(app_description).c_str(),
            base::win::ScopedMutexCreationFlag::kNone,
            base::win::security::DefaultMutexAccessRights)},
        status_{CheckStatus(app_instance_mutex_)} {
  }
#elif defined(WB_OS_POSIX)
      : app_instance_mutex_{CreateProcessMutex(app_description)},
        status_{CheckStatus(app_instance_mutex_)} {
  }
#endif

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedAppInstanceManager);

  /**
   * @brief Get application instance state.
   * @return Status.
   */
  [[nodiscard]] AppInstanceStatus GetStatus() const noexcept { return status_; }

 private:
#ifdef WB_OS_WIN
  /**
   * @brief Mutex to be acquired only by single app instance.
   */
  const base::std2::result<base::win::ScopedMutex> app_instance_mutex_;
#elif defined(WB_OS_POSIX)
  /**
   * @brief Shared memory object to be acquired only by single app instance.
   */
  const base::std2::result<base::posix::ScopedSharedMemoryObject>
      app_instance_mutex_;
#endif  // WB_OS_WIN

  /**
   * @brief App instance status.
   */
  const AppInstanceStatus status_;

  WB_ATTRIBUTE_UNUSED_FIELD
  std::array<std::byte, sizeof(char*) - sizeof(status_)> pad_;  //-V1055

  /**
   * @brief Make mutex name.
   * @param app_description App description.
   * @return Mutex name.
   */
  [[nodiscard]] static std::string MakeMutexName(
      const char* app_description) noexcept {
    G3DCHECK(!!app_description);

#ifdef WB_OS_WIN
    return "WhiteBox " + std::string{app_description} + " Singleton Mutex";
#else
    // Start with / to be portable.
    return "/WhiteBox " + std::string{app_description} + " Singleton Mutex";
#endif  // WB_OS_WIN
  }

#ifdef WB_OS_WIN
  /**
   * @brief Get app instance status.
   * @param mutex_result Mutex.
   * @return App instance status.
   */
  [[nodiscard]] static AppInstanceStatus CheckStatus(
      const base::std2::result<base::win::ScopedMutex>& mutex_result) noexcept {
    if (auto* mutex = base::std2::get_result(mutex_result))
      WB_ATTRIBUTE_LIKELY { return AppInstanceStatus::kNoOtherInstances; }

    const auto* rc = base::std2::get_error(mutex_result);
    G3DCHECK(!!rc);

    constexpr int kAccessDenied{5};
    // If the mutex is a named mutex and the object existed before this
    // function call, the return value is a handle to the existing object, and
    // the GetLastError function returns ERROR_ALREADY_EXISTS.
    constexpr int kAlreadyExists{183};

    switch (rc->value()) {
      case kAccessDenied:
      case kAlreadyExists:
        return AppInstanceStatus::kAlreadyRunning;

      default:
        G3PLOG_E(WARNING, *rc)
            << "Unable to determine either app already running or "
               "not.  Mutex creation failed.";
        return AppInstanceStatus::kUnableToDetermine;
    }
  }
#elif defined(WB_OS_POSIX)
  /**
   * Creates process shared mutex.
   * @param mutex_or_error Process shared mutex attribute or error.
   * @return Process shared mutex.
   */
  [[nodiscard]] static base::std2::result<base::posix::ScopedSharedMemoryObject>
  CreateProcessMutex(const char* app_description) noexcept {
    using namespace wb::base::posix;

    return ScopedSharedMemoryObject::New(
        MakeMutexName(app_description),
        ScopedSharedMemoryObjectFlags::kCreate |
            ScopedSharedMemoryObjectFlags::kExclusive,
        ScopedAccessModeFlags::kOwnerRead);
  };

  /**
   * @brief Get app instance status.
   * @param mutex_result Mutex.
   * @return App instance status.
   */
  [[nodiscard]] static AppInstanceStatus CheckStatus(
      const base::std2::result<base::posix::ScopedSharedMemoryObject>&
          mutex_or_error) noexcept {
    using namespace wb::base;

    if (std2::get_result(mutex_or_error)) WB_ATTRIBUTE_LIKELY {
        // Shared process mutex is locked by current process, will be unlocked
        // on process end.
        // TODO(dimhotepus): Ensure ScopedSharedMemoryObject freed on crash.
        return AppInstanceStatus::kNoOtherInstances;
      }

    const auto* error = std2::get_error(mutex_or_error);
    G3DCHECK(!!error);

    if (error->value() == EEXIST) {
      // Mutex already exists means locked by other instance.
      return AppInstanceStatus::kAlreadyRunning;
    }

    G3PLOG_E(WARNING, *error)
        << "Unable to determine either app already running or "
           "not.  Mutex creation failed.";
    return AppInstanceStatus::kUnableToDetermine;
  }
#endif  // WB_OS_WIN
};

}  // namespace wb::boot_manager

#endif  // !WB_WHITEBOX_BOOT_MANAGER_SCOPED_APP_INSTANCE_MANAGER_H_