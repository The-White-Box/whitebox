// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Ensures single app instance is running.

#ifndef WB_BASE_SCOPED_APP_INSTANCE_MANAGER_H_
#define WB_BASE_SCOPED_APP_INSTANCE_MANAGER_H_

#include <array>

#include "base/config.h"
#include "base/macroses.h"
#include "base/std2/system_error_ext.h"
#include "build/build_config.h"

#ifdef WB_OS_WIN
#include "base/win/scoped_mutex.h"
#elif defined(WB_OS_POSIX)
#include "base/posix/scoped_shared_memory_object.h"
#endif

namespace wb::base {

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
class WB_BASE_API ScopedAppInstanceManager {
 public:
  /**
   * @brief Create app instance manager.
   * @param app_description Application description.
   * @return nothing.
   */
  explicit ScopedAppInstanceManager(const char* app_description) noexcept;

  ~ScopedAppInstanceManager() noexcept = default;

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedAppInstanceManager);

  /**
   * @brief Get application instance state.
   * @return Status.
   */
  [[nodiscard]] AppInstanceStatus GetStatus() const noexcept { return status_; }

 private:
#ifdef WB_OS_WIN
  WB_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()
    // Private member is not accessible to the DLL's client, including inline
    // functions.
    WB_MSVC_DISABLE_WARNING(4251)
    /**
     * @brief Mutex to be acquired only by single app instance.
     */
    const std2::result<win::ScopedMutex> app_instance_mutex_;
  WB_MSVC_END_WARNING_OVERRIDE_SCOPE()
#elif defined(WB_OS_POSIX)
  /**
   * @brief Shared memory object to be acquired only by single app instance.
   */
  const std2::result<posix::ScopedSharedMemoryObject> app_instance_mutex_;
#endif  // WB_OS_WIN

  /**
   * @brief App instance status.
   */
  const AppInstanceStatus status_;

  WB_ATTRIBUTE_UNUSED_FIELD
  std::array<std::byte, sizeof(char*) - sizeof(status_)> pad_;  //-V1055
};

}  // namespace wb::base

#endif  // !WB_BASE_SCOPED_APP_INSTANCE_MANAGER_H_