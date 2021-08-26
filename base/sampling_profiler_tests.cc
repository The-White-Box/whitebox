// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Simple sampling profiler.

#include "sampling_profiler.h"
//
#include <chrono>
#include <thread>
#include <type_traits>
//
#include "base/deps/googletest/gtest/gtest.h"

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(SamplingProfilerTest, UsingClockTimepointDurationFromArg) {
  static_assert(std::is_same_v<
                std::chrono::system_clock,
                wb::base::SamplingProfiler<std::chrono::system_clock>::clock>);
  static_assert(
      std::is_same_v<
          std::chrono::system_clock::time_point,
          wb::base::SamplingProfiler<std::chrono::system_clock>::time_point>);
  static_assert(
      std::is_same_v<
          std::chrono::system_clock::time_point::duration,
          wb::base::SamplingProfiler<std::chrono::system_clock>::duration>);
}

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(SamplingProfilerTest, NoCopyConstructorAndAssignment) {
  static_assert(!std::is_copy_constructible_v<
                wb::base::SamplingProfiler<std::chrono::system_clock>>);
  static_assert(!std::is_copy_assignable_v<
                wb::base::SamplingProfiler<std::chrono::system_clock>>);
}

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(HighResolutionSamplingProfilerTest, MoveConstructorMovesState) {
  wb::base::HighResolutionSamplingProfiler profiler{
      std::chrono::steady_clock::now()};

  using namespace std::chrono_literals;
  // Well, windows sleep precision can be not so high.  Can be changed by
  // ScopedMinimumTimerResolution, but can affect other processes.  Decided to
  // use a bit larger sleep time than the default precision to ensure test
  // passes.
  constexpr auto average_sample_time{15ms};
  // Allow 25% deviation for testing host CPU fluctuations.
  constexpr auto sample_time_deviation{average_sample_time / 4};

  auto moved_profiler = std::move(profiler);

  {
    std::this_thread::sleep_for(average_sample_time);
    moved_profiler.Sample();

    const auto time_delta = moved_profiler.GetTimeBetweenLastSamples();

    EXPECT_GE(time_delta, average_sample_time - sample_time_deviation)
        << "Should sample " << average_sample_time.count() << "ms or more.";
    EXPECT_LT(time_delta, average_sample_time + sample_time_deviation)
        << "Deviation should be " << sample_time_deviation.count()
        << "ms or less";
  }
}

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(HighResolutionSamplingProfilerTest,
           SampleAndGetTimeBetweenLastSamples) {
  wb::base::HighResolutionSamplingProfiler profiler{
      std::chrono::steady_clock::now()};

  using namespace std::chrono_literals;
  // Well, windows sleep precision can be not so high.  Can be changed by
  // ScopedMinimumTimerResolution, but can affect other processes.  Decided to
  // use a bit larger sleep time than the default precision to ensure test
  // passes.
  constexpr auto average_sample_time{15ms};
  // Allow 25% deviation for testing host CPU fluctuations.
  constexpr auto sample_time_deviation{average_sample_time / 4};

  {
    std::this_thread::sleep_for(average_sample_time);
    profiler.Sample();

    const auto time_delta = profiler.GetTimeBetweenLastSamples();

    EXPECT_GE(time_delta, average_sample_time - sample_time_deviation)
        << "Should sample " << average_sample_time.count() << "ms or more.";
    EXPECT_LT(time_delta, average_sample_time + sample_time_deviation)
        << "Deviation should be " << sample_time_deviation.count()
        << "ms or less";
  }

  {
    std::this_thread::sleep_for(average_sample_time);
    profiler.Sample();

    const auto time_delta = profiler.GetTimeBetweenLastSamples();

    EXPECT_GE(time_delta, average_sample_time - sample_time_deviation)
        << "Should sample " << average_sample_time.count() << "ms or more.";
    EXPECT_LT(time_delta, average_sample_time + sample_time_deviation)
        << "Deviation should be " << sample_time_deviation.count()
        << "ms or less";
  }

  {
    std::this_thread::sleep_for(average_sample_time);
    profiler.Sample();

    const auto time_delta = profiler.GetTimeBetweenLastSamples();

    EXPECT_GE(time_delta, average_sample_time - sample_time_deviation)
        << "Should sample " << average_sample_time.count() << "ms or more.";
    EXPECT_LT(time_delta, average_sample_time + sample_time_deviation)
        << "Deviation should be " << sample_time_deviation.count()
        << "ms or less";
  }
}