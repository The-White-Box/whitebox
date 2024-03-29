// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Base command line flags.

#ifndef WB_APPS_BASE_FLAGS_H_
#define WB_APPS_BASE_FLAGS_H_

#include <cstdint>      // uint32_t
#include <string>       // string
#include <string_view>  // string_view

#include "base/deps/abseil/flags/declare.h"  // ABSL_DECLARE_FLAG
#include "build/build_config.h"              // WB_OS_WIN

namespace wb::apps::flags {

#ifdef WB_OS_WIN
/**
 * @brief Periodic timer resolution.
 */
struct PeriodicTimerResolution {
  explicit PeriodicTimerResolution(uint32_t ms_) noexcept : ms{ms_} {}

  /**
   * @brief Resolution in milliseconds.
   */
  uint32_t ms;  // Valid range is [1..XXXXXX]
};

/**
 * @brief Returns a textual flag value corresponding to the
 * PeriodicTimerResolution.
 * @param p PeriodicTimerResolution
 * @return Textual flag value.
 */
std::string AbslUnparseFlag(PeriodicTimerResolution p);

/**
 * @brief Parses a PeriodicTimerResolution from the command line flag value
 * `text.
 * @param text Command line flag value.
 * @param p PeriodicTimerResolution.
 * @param error Parse flag error.
 * @return true and sets `*p` on success; returns false and sets `*error` on
 * failure.
 */
bool AbslParseFlag(std::string_view text, PeriodicTimerResolution* p,
                   std::string* error);
#endif  // WB_OS_WIN

/**
 * @brief Window size.
 */
struct WindowSize {
  explicit WindowSize(std::uint16_t size_) noexcept : size{size_} {}

  /**
   * @brief Size in pixels.
   */
  std::uint16_t size;

  WindowSize(const WindowSize&) noexcept = default;
  WindowSize(WindowSize&&) noexcept = default;
  WindowSize& operator=(const WindowSize&) noexcept = default;
  WindowSize& operator=(WindowSize&&) noexcept = default;
};

/**
 * @brief Window width.
 */
struct WindowWidth : WindowSize {
  explicit WindowWidth(std::uint16_t size_) noexcept : WindowSize{size_} {}
};

/**
 * @brief Returns a textual flag value corresponding to the WindowWidth.
 * @param w WindowWidth.
 * @return Textual flag value.
 */
std::string AbslUnparseFlag(WindowWidth w);

/**
 * @brief Parses a WindowWidth from the command line flag value `text`.
 * @param text Command line flag value.
 * @param w WindowDimension.
 * @param error Parse flag error.
 * @return true and sets `*p` on success; returns false and sets `*error` on
 * failure.
 */
bool AbslParseFlag(std::string_view text, WindowWidth* w, std::string* error);

/**
 * @brief Window height.
 */
struct WindowHeight : WindowSize {
  explicit WindowHeight(std::uint16_t size_) noexcept : WindowSize{size_} {}
};

/**
 * @brief Returns a textual flag value corresponding to the WindowHeight.
 * @param h WindowHeight.
 * @return Textual flag value.
 */
std::string AbslUnparseFlag(WindowHeight h);

/**
 * @brief Parses a WindowHeight from the command line flag value `text`.
 * @param text Command line flag value.
 * @param h WindowDimension.
 * @param error Parse flag error.
 * @return true and sets `*p` on success; returns false and sets `*error` on
 * failure.
 */
bool AbslParseFlag(std::string_view text, WindowHeight* h, std::string* error);

/**
 * @brief Assets path.
 */
struct AssetsPath {
  explicit AssetsPath(std::string value_) noexcept : value{std::move(value_)} {}

  std::string value;
};

/**
 * @brief Returns a textual flag value corresponding to the AssetsPath.
 * @param p AssetsPath.
 * @return Textual flag value.
 */
std::string AbslUnparseFlag(AssetsPath p);

/**
 * @brief Parses a AssetsPath from the command line flag value `text`.
 * @param text Command line flag value.
 * @param p AssetsPath.
 * @param error Parse flag error.
 * @return true and sets `*p` on success; returns false and sets `*error` on
 * failure.
 */
bool AbslParseFlag(std::string_view text, AssetsPath* p, std::string* error);

}  // namespace wb::apps::flags

// Assets path.
ABSL_DECLARE_FLAG(wb::apps::flags::AssetsPath, assets_path);

// How many memory cleanup & reallocation attempts to do when out of memory.
ABSL_DECLARE_FLAG(std::uint32_t, attempts_to_retry_allocate_memory);

#ifdef WB_OS_WIN
// Insecure.  Allow to load NOT SIGNED module targets.  There is no guarantee
// unsigned module doing nothing harmful.  Use at your own risk, ex. for
// debugging or mods.
ABSL_DECLARE_FLAG(bool, insecure_allow_unsigned_module_target);
#endif  // WB_OS_WIN

// Initial width of the main window in pixels.
ABSL_DECLARE_FLAG(wb::apps::flags::WindowWidth, main_window_width);

// Initial height of the main window in pixels.
ABSL_DECLARE_FLAG(wb::apps::flags::WindowHeight, main_window_height);

#ifdef WB_OS_WIN
// Changes minimal resolution (ms) of the Windows periodic timer.  Setting a
// higher resolution can improve the accuracy of time-out intervals in wait
// functions.  However, it can also reduce overall system performance, because
// the thread scheduler switches tasks more often.  High resolutions can also
// prevent the CPU power management system from entering power-saving modes.
// Setting a higher resolution does not improve the accuracy of the
// high-resolution performance counter.
ABSL_DECLARE_FLAG(wb::apps::flags::PeriodicTimerResolution,
                  periodic_timer_resolution_ms);
#endif  // WB_OS_WIN

// Should dump heap allocator statistics on exit or not.  Included some process
// info, like system / user elapsed time, peak working set size, hard page
// faults, etc.
ABSL_DECLARE_FLAG(bool, should_dump_heap_allocator_statistics_on_exit);

#endif  // !WB_APPS_BASE_FLAGS_H_
