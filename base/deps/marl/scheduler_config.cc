// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// marl scheduler configuration.

#include "base/deps/marl/scheduler_config.h"

#include "base/deps/abseil/strings/str_cat.h"
#include "base/deps/g3log/g3log.h"
#include "base/std2/thread_ext.h"

#ifdef WB_OS_WIN
#include "base/win/com/scoped_thread_com_initializer.h"
#include "base/win/error_handling/scoped_thread_error_mode.h"
#include "base/win/error_handling/scoped_thread_invalid_parameter_handler.h"
#endif

namespace wb::base::deps::marl {

/**
 * @brief Whitebox thread start state implementation.
 */
class WhiteboxThreadStartState::Impl final {
 public:
  Impl(int workerId) noexcept
      : scoped_thread_name_{std2::this_thread::ScopedThreadName::New(
            absl::StrCat(kThreadNamePrefix, workerId))}
#ifdef WB_OS_WIN
        ,
        scoped_thread_error_mode_{
            win::error_handling::ScopedThreadErrorMode::New(
#ifdef NDEBUG
                win::error_handling::ScopedThreadErrorModeFlags::
                    kFailOnCriticalErrors |
#endif
                win::error_handling::ScopedThreadErrorModeFlags::
                    kNoGpFaultErrorBox |
                win::error_handling::ScopedThreadErrorModeFlags::
                    kNoOpenFileErrorBox)},
        scoped_thread_invalid_parameter_handler_{
            win::error_handling::DefaultThreadInvalidParameterHandler},
        scoped_thread_com_initializer_{
            win::com::ScopedThreadComInitializer::New(
                win::com::ScopedThreadComInitializerFlags::kApartmentThreaded |
                win::com::ScopedThreadComInitializerFlags::kDisableOle1Dde |
                win::com::ScopedThreadComInitializerFlags::kSpeedOverMemory)}
#endif
  {
#ifdef WB_OS_WIN
    // SIGFPE, SIGILL, and SIGSEGV handling must be installed per thread on
    // Windows.
    g3::installSignalHandlerForThread();
#endif

    G3PLOGE2_IF(WARNING, scoped_thread_name_.error_or(std2::ok_code))
        << "Can't rename worker thread #" << workerId
        << ", continue with default name.";

#ifdef WB_OS_WIN
    G3PLOGE2_IF(WARNING, scoped_thread_error_mode_.error_or(std2::ok_code))
        << "Can't set reaction to serious system errors for worker thread #"
        << workerId << ", continue with default reaction.";
    G3PLOGE2_IF(WARNING, scoped_thread_com_initializer_.error_or(std2::ok_code))
        << "Component Object Model initialization failed for worker thread #"
        << workerId << ", continue without COM.";
#endif
  }

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(Impl);

  ~Impl() noexcept = default;

 private:
  template <typename T>
  using r = std2::result<T>;

  const r<std2::this_thread::ScopedThreadName> scoped_thread_name_;

#ifdef WB_OS_WIN
  // Calling thread will handle critical errors, does not show general
  // protection fault error box and message box when OpenFile failed to find
  // file.
  const r<win::error_handling::ScopedThreadErrorMode> scoped_thread_error_mode_;
  // Handle call of CRT function with bad arguments on the thread.
  const win::error_handling::ScopedThreadInvalidParameterHandler
      scoped_thread_invalid_parameter_handler_;
  const r<win::com::ScopedThreadComInitializer> scoped_thread_com_initializer_;
#endif

  const static std2::native_thread_name kThreadNamePrefix;
};

const std2::native_thread_name
    WhiteboxThreadStartState::Impl::kThreadNamePrefix = "WhiteBox_Worker#";

WhiteboxThreadStartState::WhiteboxThreadStartState(int workerId) noexcept
    : impl_{std::make_unique<Impl>(workerId)} {}

WhiteboxThreadStartState::~WhiteboxThreadStartState() noexcept = default;

WB_BASE_API std::unique_ptr<::marl::Thread::StartState> make_thread_start_state(
    int workerId) {
  return std::make_unique<WhiteboxThreadStartState>(workerId);
}

}  // namespace wb::base::deps::marl
