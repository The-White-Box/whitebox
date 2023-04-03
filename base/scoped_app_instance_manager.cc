// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Ensures single app instance is running.

#include "scoped_app_instance_manager.h"

#include <string>

#include "base/deps/abseil/strings/str_cat.h"

namespace {

/**
 * @brief Make mutex name.
 * @param app_description App description.
 * @return Mutex name.
 */
[[nodiscard]] std::string MakeMutexName(const char* app_description) noexcept {
  G3DCHECK(!!app_description);

  return absl::StrCat(
#ifdef WB_OS_WIN
      "WhiteBox "
#else   // WB_OS_WIN
      // Start with / to be portable.
      "/WhiteBox "
#endif  // !WB_OS_WIN
      ,
      app_description, " Singleton Mutex");
}

#ifdef WB_OS_WIN
/**
 * @brief Get app instance status.
 * @param mutex_result Mutex.
 * @return App instance status.
 */
[[nodiscard]] wb::base::AppInstanceStatus CheckStatus(
    const wb::base::std2::result<wb::base::win::ScopedMutex>&
        mutex_result) noexcept {
  using namespace wb::base;

  if (auto* mutex = std2::get_result(mutex_result)) [[likely]] {
    return AppInstanceStatus::kNoOtherInstances;
  }

  const auto* rc = std2::get_error(mutex_result);
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
[[nodiscard]] wb::base::std2::result<wb::base::posix::ScopedSharedMemoryObject>
CreateProcessMutex(const char* app_description) noexcept {
  using namespace wb::base::posix;

  return ScopedSharedMemoryObject::New(
      MakeMutexName(app_description),
      ScopedSharedMemoryObjectFlags::kCreate |
          ScopedSharedMemoryObjectFlags::kExclusive |
          ScopedSharedMemoryObjectFlags::kReadWrite,
      ScopedAccessModeFlags::kOwnerRead);
}

/**
 * @brief Get app instance status.
 * @param mutex_result Mutex.
 * @return App instance status.
 */
[[nodiscard]] wb::base::AppInstanceStatus CheckStatus(
    const wb::base::std2::result<wb::base::posix::ScopedSharedMemoryObject>&
        mutex_or_error) noexcept {
  using namespace wb::base;

  if (std2::get_result(mutex_or_error)) [[likely]] {
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

}  // namespace

namespace wb::base {

ScopedAppInstanceManager::ScopedAppInstanceManager(
    const char* app_description) noexcept
#ifdef WB_OS_WIN
    : app_instance_mutex_{win::ScopedMutex::New(
          nullptr, MakeMutexName(app_description).c_str(),
          win::ScopedMutexCreationFlag::kNone,
          win::security::DefaultMutexAccessRights)},
      status_{CheckStatus(app_instance_mutex_)} {}
#elif defined(WB_OS_POSIX)
    : app_instance_mutex_{CreateProcessMutex(app_description)},
      status_{CheckStatus(app_instance_mutex_)} {
}
#endif

}  // namespace wb::base