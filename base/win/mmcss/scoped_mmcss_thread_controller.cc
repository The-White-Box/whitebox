// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Controls Multimedia Class Scheduler service (MMCSS) thread settings.  Ensure
// multimedia applications having time-sensitive processing receive prioritized
// access to CPU resources.
//
// MMCSS uses information stored in the registry to identify supported tasks and
// determine the relative priority of threads performing these tasks.  Each
// thread that is performing work related to a particular task calls the
// AvSetMmMaxThreadCharacteristics or AvSetMmThreadCharacteristics function to
// inform MMCSS that it is working on that task.

#include "scoped_mmcss_thread_controller.h"

#include <algorithm>  // std::clamp

#include "base/deps/g3log/g3log.h"
#include "base/win/system_error_ext.h"
#include "base/win/windows_light.h"
//
#include <avrt.h>

namespace wb::base::windows::mmcss {
class ScopedMmcssThreadController::ScopedMmcssThreadControllerImpl {
 public:
  ScopedMmcssThreadControllerImpl(
      const ScopedMmcssThreadTask& first_task,
      const ScopedMmcssThreadTask& last_task) noexcept;

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedMmcssThreadControllerImpl);

  ~ScopedMmcssThreadControllerImpl() noexcept;

  [[nodiscard]] std::error_code error_code() const noexcept {
    return error_code_;
  }

  std_ext::os_res<unsigned char> GetResponsivenessPercent() const noexcept;

  std::error_code SetPriority(
      ScopedMmcssThreadPriority priority) const noexcept;

 private:
  /**
   * @brief The unique task identifier.
   */
  unsigned long task_index_;

  [[maybe_unused]] std::byte pad_[sizeof(char*) - sizeof(task_index_)];

  /**
   * @brief Task handle.
   */
  const HANDLE task_handle_;

  /**
   * @brief Initialization error code.
   */
  const std::error_code error_code_;
};

ScopedMmcssThreadController::ScopedMmcssThreadControllerImpl::
    ScopedMmcssThreadControllerImpl(
        const ScopedMmcssThreadTask& first_task,
        const ScopedMmcssThreadTask& last_task) noexcept
    : task_index_{0},
      task_handle_{::AvSetMmMaxThreadCharacteristicsA(
          first_task.name(), last_task.name(), &task_index_)},
      error_code_{task_handle_ ? std::error_code{}
                               : std_ext::GetThreadErrorCode()} {
  // Well, if smb removed task from registry or no privileges it is ok.
  G3DCHECK(!error_code_ || error_code_.value() == ERROR_INVALID_TASK_NAME ||
           error_code_.value() == ERROR_PRIVILEGE_NOT_HELD);
}

ScopedMmcssThreadController::ScopedMmcssThreadControllerImpl::
    ~ScopedMmcssThreadControllerImpl() noexcept {
  if (task_handle_) {
    const auto rc =
        GetErrorCode(::AvRevertMmThreadCharacteristics(task_handle_));
    G3PCHECK_E(!rc, rc) << "AvRevertMmThreadCharacteristics failed";
  }
}  // namespace wb::base::windows::mmcss

std_ext::os_res<unsigned char> ScopedMmcssThreadController::
    ScopedMmcssThreadControllerImpl::GetResponsivenessPercent() const noexcept {
  G3DCHECK(!!task_handle_);

  unsigned long responsiveness_percent{0};
  const auto rc = GetErrorCode(
      ::AvQuerySystemResponsiveness(task_handle_, &responsiveness_percent));

  G3DPCHECK_E(!rc, rc) << "AvQuerySystemResponsiveness failed";

  if (!rc) [[likely]] {
    const bool is_valid_responsiveness{responsiveness_percent >= 10UL &&
                                       responsiveness_percent <= 100UL};
    // Expected [10..100] range, but OS may be buggy.  See
    // https://docs.microsoft.com/en-us/windows/win32/api/avrt/nf-avrt-avquerysystemresponsiveness#parameters
    G3DCHECK(is_valid_responsiveness)
        << "AvQuerySystemResponsiveness returned system responsiveness percent "
           "not in [10..100] range ("
        << responsiveness_percent << "%).";
    if (!is_valid_responsiveness) [[unlikely]] {
      responsiveness_percent = std::clamp(responsiveness_percent, 10UL, 100UL);
    }
    return std_ext::os_res<unsigned char>{
        static_cast<unsigned char>(responsiveness_percent)};
  }

  return std_ext::os_res<unsigned char>{rc};
}

std::error_code
ScopedMmcssThreadController::ScopedMmcssThreadControllerImpl::SetPriority(
    ScopedMmcssThreadPriority priority) const noexcept {
  G3DCHECK(!!task_handle_);

  const auto native_priority = enum_cast<AVRT_PRIORITY>(priority);
  const auto rc =
      GetErrorCode(::AvSetMmThreadPriority(task_handle_, native_priority));

  G3DPCHECK_E(!rc, rc) << "AvSetMmThreadPriority failed";

  return rc;
}

std_ext::os_res<ScopedMmcssThreadController> ScopedMmcssThreadController::New(
    const ScopedMmcssThreadTask& first_task,
    const ScopedMmcssThreadTask& last_task) noexcept {
  ScopedMmcssThreadController controller{first_task, last_task};
  return !controller.error_code()
             ? std_ext::os_res<ScopedMmcssThreadController>{std::move(
                   controller)}
             : std_ext::os_res<ScopedMmcssThreadController>{
                   controller.error_code()};
}

ScopedMmcssThreadController::ScopedMmcssThreadController(
    const ScopedMmcssThreadTask& first_task,
    const ScopedMmcssThreadTask& last_task) noexcept
    : impl_{std::make_unique<ScopedMmcssThreadControllerImpl>(first_task,
                                                              last_task)} {}

[[nodiscard]] std::error_code ScopedMmcssThreadController::error_code()
    const noexcept {
  return impl_->error_code();
}

ScopedMmcssThreadController::ScopedMmcssThreadController(
    ScopedMmcssThreadController&& p) noexcept = default;

ScopedMmcssThreadController::~ScopedMmcssThreadController() noexcept = default;

std_ext::os_res<unsigned char>
ScopedMmcssThreadController::GetResponsivenessPercent() const noexcept {
  return impl_->GetResponsivenessPercent();
}

std::error_code ScopedMmcssThreadController::SetPriority(
    ScopedMmcssThreadPriority priority) const noexcept {
  return impl_->SetPriority(priority);
}
}  // namespace wb::base::windows::mmcss
