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
#include "base/deps/abseil/cleanup/cleanup.h"

namespace wb::base::deps::marl {

namespace internal {

template <typename Tuple, size_t... Indices>
static void Invoke(void* raw_values) noexcept {
  const std::unique_ptr<Tuple> fn_values{static_cast<Tuple*>(raw_values)};
  Tuple& tuple{*fn_values.get()};
  std::invoke(std::move(std::get<Indices>(tuple))...);
}

template <typename Tuple, size_t... Indices>
[[nodiscard]] static constexpr auto GetInvoke(
    std::index_sequence<Indices...>) noexcept {
  return &Invoke<Tuple, Indices...>;
}

}  // namespace internal

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
template <typename Function, typename... Args>
[[nodiscard]] std::future<
    std::invoke_result_t<std::decay_t<Function>, std::decay_t<Args>...>>
schedule(Function&& f, Args&&... args) {
  using Result =
      std::invoke_result_t<std::decay_t<Function>, std::decay_t<Args>...>;

  std::packaged_task<Result(Args...)> task{std::forward<Function>(f)};
  std::future<Result> future = task.get_future();

  auto lambda = [t = std::move(task)](Args&&... lambda_args) mutable {
    t(std::forward<Args>(lambda_args)...);
  };

  using Lambda = std::decay_t<decltype(lambda)>;
  using Tuple = std::tuple<Lambda, std::decay_t<Args>...>;

  constexpr auto invoke = internal::GetInvoke<Tuple>(
      std::make_index_sequence<1 + sizeof...(Args)>{});
  auto invoke_state = std::make_unique<Tuple>(std::forward<Lambda>(lambda),
                                              std::forward<Args>(args)...);
  const absl::Cleanup release_state{[&]() noexcept {
    // Owned by scheduler thread now.
    invoke_state.release();
  }};

  ::marl::schedule(invoke, invoke_state.get());

  return future;
}

}  // namespace wb::base::deps::marl

#endif  // !WB_BASE_DEPS_MARL_SCHEDULER_H_
