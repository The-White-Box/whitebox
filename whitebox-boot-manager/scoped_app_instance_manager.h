// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Ensures single app instance is running.

#ifndef WB_WHITEBOX_BOOT_MANAGER_SCOPED_APP_INSTANCE_MANAGER_H_
#define WB_WHITEBOX_BOOT_MANAGER_SCOPED_APP_INSTANCE_MANAGER_H_

#include <array>
#include <string>

#include "base/base_macroses.h"
#include "base/std2/system_error_ext.h"
#include "build/build_config.h"

#ifdef WB_OS_WIN
#include "base/win/scoped_mutex.h"
#elif defined(WB_OS_LINUX)

#elif defined(WB_OS_MAC)

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
      : app_instance_mutex_{base::windows::ScopedMutex::New(
            nullptr, MakeMutexName(app_description).c_str(),
            base::windows::ScopedMutexCreationFlag::kNone,
            base::windows::security::DefaultMutexAccessRights)},
        status_{CheckStatus(app_instance_mutex_)} {
  }
#else
      : status_{AppInstanceStatus::kUnableToDetermine} {
    // TODO(dimhotepus): Use lock file in user app data dir.
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
  const base::std2::result<base::windows::ScopedMutex> app_instance_mutex_;
#endif

  /**
   * @brief APp instance status.
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
    return "WhiteBox " + std::string{app_description} + " Singleton Mutex";
  }

#ifdef WB_OS_WIN
  /**
   * @brief Get app instance status.
   * @param mutex_result Mutex.
   * @return App instance status.
   */
  [[nodiscard]] static AppInstanceStatus CheckStatus(
      const base::std2::result<base::windows::ScopedMutex>&
          mutex_result) noexcept {
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
#endif
};

}  // namespace wb::boot_manager

#endif  // !WB_WHITEBOX_BOOT_MANAGER_SCOPED_APP_INSTANCE_MANAGER_H_