// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// marl scheduler.h wrapper.

#ifndef WB_BASE_DEPS_MARL_SCHEDULER_H_
#define WB_BASE_DEPS_MARL_SCHEDULER_H_

#include "base/deps/marl/marl_config.h"

WB_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()
  WB_MSVC_DISABLE_WARNING(4355)
  WB_MSVC_DISABLE_WARNING(5204)
#include <future>
WB_MSVC_END_WARNING_OVERRIDE_SCOPE()

WB_BEGIN_MARL_WARNING_OVERRIDE_SCOPE()
#include "deps/marl/include/marl/scheduler.h"
WB_END_MARL_WARNING_OVERRIDE_SCOPE()

namespace wb::base::deps::marl {

/**
 * @brief schedule() schedules the function f to be asynchronously called with
 * the given arguments using the currently bound scheduler and provides future
 * with result.
 * @tparam Function Function.
 * @tparam ...Args Arguments.
 * @param f Function.
 * @param ...args Arguments.
 * @return Future with function result.
 */
// template <typename Function, typename... Args>
//[[nodiscard]] std::future<
//     std::invoke_result_t<std::decay_t<Function>, std::decay_t<Args>...>>
// schedule(Function&& f, Args&&... args) {
//   using ScheduleResult =
//       std::invoke_result_t<std::decay_t<Function>, std::decay_t<Args>...>;
//
//   std::packaged_task task{std::forward<Function>(f)};
//   std::future<ScheduleResult> future = task.get_future();
//
//   ::marl::schedule(std::move(task), std::forward<Args>(args)...);
//
//   return future;
// }

}  // namespace wb::base::deps::marl

#endif  // !WB_BASE_DEPS_MARL_SCHEDULER_H_
