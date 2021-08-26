// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Simple sampling profiler.

#ifndef WB_BASE_SAMPLING_PROFILER_H_
#define WB_BASE_SAMPLING_PROFILER_H_

#include <array>
#include <chrono>  // std::chrono::steady_clock

#include "base/base_macroses.h"

namespace wb::base {
/**
 * @brief Sampling profiler.
 * @tparam TClock Clock to use as time source.
 */
template <typename TClock>
class SamplingProfiler {
 public:
  /**
   * @brief Clock.
   */
  using clock = TClock;
  /**
   * @brief Time point.
   */
  using time_point = std::chrono::time_point<clock>;
  /**
   * @brief Time duration.
   */
  using duration = typename time_point::duration;

  /**
   * @brief Creates sampling profiler.
   * @param initial_time Initial time to use as first sample point.
   * @return nothing.
   */
  explicit SamplingProfiler(time_point initial_time) noexcept
      : sample_times_{initial_time, initial_time},
        last_written_sample_idx_{0U} {}

  SamplingProfiler(SamplingProfiler &&p) noexcept = default;
  SamplingProfiler &operator=(SamplingProfiler &&p) noexcept = default;

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(SamplingProfiler);

  /**
   * @brief Grabs time sample.
   * @return void.
   */
  void Sample() noexcept {
    last_written_sample_idx_ =
        (last_written_sample_idx_ + 1) % sample_times_.size();
    sample_times_[last_written_sample_idx_] = clock::now();
  }

  /**
   * @brief Gets time between last samples.
   * @return Time between last samples.  Note, time may be zero!
   */
  [[nodiscard]] duration GetTimeBetweenLastSamples() const noexcept {
    return sample_times_[last_written_sample_idx_] -
           sample_times_[GetPreviousSampleIdx()];
  }

 private:
  /**
   * @brief Sample time points.
   */
  std::array<time_point, 2> sample_times_;
  /**
   * @brief Last written sample index.
   */
  std::size_t last_written_sample_idx_;

  /**
   * @brief Get previous sample position index.
   * @return Previous sample position index.
   */
  [[nodiscard]] std::size_t GetPreviousSampleIdx() const noexcept {
    return (last_written_sample_idx_ - 1) % sample_times_.size();
  }
};

/**
 * @brief High resolution sampling profiler.  Do not use high_resolution_clock
 * here, as it is not implemented consistently across different standard library
 * implementations, and its use should be avoided.  It is often just an alias
 * for std::chrono::steady_clock or std::chrono::system_clock, but which one it
 * is depends on the library or configuration.
 *
 * Class std::chrono::steady_clock represents a monotonic clock and most
 * suitable for measuring intervals.
 */
using HighResolutionSamplingProfiler =
    SamplingProfiler<std::chrono::steady_clock>;
}  // namespace wb::base

#endif  // !WB_BASE_SAMPLING_PROFILER_H_