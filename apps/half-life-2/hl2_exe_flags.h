// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Half-Life 2 executable command line flags.

#ifndef WB_APPS_HALF_LIFE_2_HL2_EXE_FLAGS_H_
#define WB_APPS_HALF_LIFE_2_HL2_EXE_FLAGS_H_

#include <cstdint>      // uint32_t
#include <string>       // string
#include <string_view>  // string_view

#include "app_version_config.h"
#include "base/deps/abseil/flags/declare.h"  // ABSL_DECLARE_FLAG
#include "build/build_config.h"              // WB_OS_WIN

namespace wb::apps::half_life_2 {

/**
 * @brief Usage message.
 */
constexpr char kUsageMessage[] = WB_PRODUCT_FILE_DESCRIPTION_STRING
    ".  The player again picks up the crowbar of research\nscientist Gordon "
    "Freeman, who finds himself on an alien-infested Earth being\npicked to "
    "the bone, its resources depleted, its populace dwindling.  Freeman is\n"
    "thrust into the unenviable role of rescuing the world from the wrong "
    "he\nunleashed back at Black Mesa.  And a lot of people he cares about are "
    "counting\non him.\n\nSample usage:\n";

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
 * `text`.
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
 * @param w WindowWidth
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
 * @param h WindowHeight
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

}  // namespace wb::apps::half_life_2

// How many memory cleanup & reallocation attempts to do when out of memory.
ABSL_DECLARE_FLAG(std::uint32_t, attempts_to_retry_allocate_memory);

// Initial width of the main window in pixels.
ABSL_DECLARE_FLAG(wb::apps::half_life_2::WindowWidth, main_window_width);

// Initial height of the main window in pixels.
ABSL_DECLARE_FLAG(wb::apps::half_life_2::WindowHeight, main_window_height);

#ifdef WB_OS_WIN
// Insecure.  Allow to load NOT SIGNED module targets.  There is no guarantee
// unsigned module doing nothing harmful.  Use at your own risk, ex. for
// debugging or mods.
ABSL_DECLARE_FLAG(bool, insecure_allow_unsigned_module_target);

// Changes minimal resolution (ms) of the Windows periodic timer.  Setting a
// higher resolution can improve the accuracy of time-out intervals in wait
// functions.  However, it can also reduce overall system performance, because
// the thread scheduler switches tasks more often.  High resolutions can also
// prevent the CPU power management system from entering power-saving modes.
// Setting a higher resolution does not improve the accuracy of the
// high-resolution performance counter.
ABSL_DECLARE_FLAG(wb::apps::half_life_2::PeriodicTimerResolution,
                  periodic_timer_resolution_ms);
#endif  // WB_OS_WIN

#endif  // !WB_APPS_HALF_LIFE_2_HL2_EXE_FLAGS_H_
