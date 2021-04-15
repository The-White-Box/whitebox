// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// marl scheduler configuration.

#ifndef WB_BASE_DEPS_MARL_SCHEDULER_CONFIG_H_
#define WB_BASE_DEPS_MARL_SCHEDULER_CONFIG_H_

#include <memory>

#include "base/config.h"
#include "base/deps/marl/thread.h"
#include "base/macroses.h"
#include "build/compiler_config.h"

namespace wb::base::deps::marl {

/**
 * @brief Setup thread start state.
 */
WB_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()
  WB_MSVC_DISABLE_WARNING(4275)
  class WB_BASE_API WhiteboxThreadStartState
      : public ::marl::Thread::StartState {
   public:
    WhiteboxThreadStartState(int workerId) noexcept;
    virtual ~WhiteboxThreadStartState() noexcept = default;

    WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(WhiteboxThreadStartState);

   private:
    class Impl;

    WB_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()
      // Private member is not accessible to the DLL's client, including inline
      // functions.
      WB_MSVC_DISABLE_WARNING(4251)
      wb::base::un<Impl> impl_;
    WB_MSVC_END_WARNING_OVERRIDE_SCOPE()
  };
WB_MSVC_END_WARNING_OVERRIDE_SCOPE()

/**
 * @brief Make initial state for thread.
 */
WB_BASE_API std::unique_ptr<::marl::Thread::StartState> make_thread_start_state(
    int workerId);

}  // namespace wb::base::deps::marl

#endif  // !WB_BASE_DEPS_MARL_SCHEDULER_CONFIG_H_
