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

#ifndef WB_BASE_WIN_MMCSS_SCOPED_MMCSS_THREAD_CONTROLLER_H_
#define WB_BASE_WIN_MMCSS_SCOPED_MMCSS_THREAD_CONTROLLER_H_

#include "base/base_api.h"
#include "base/base_macroses.h"
#include "base/deps/g3log/g3log.h"
#include "base/std2/system_error_ext.h"
#include "build/compiler_config.h"

namespace wb::base::windows::mmcss {
/**
 * @brief Known MMCSS thread tasks.  See
 * https://docs.microsoft.com/en-us/windows/win32/procthread/multimedia-class-scheduler-service#registry-settings
 */
struct KnownScopedMmcssThreadTaskName {
  /**
   * @brief Thread processes audio on a high level.
   */
  static inline constexpr char kAudio[6]{"Audio"};
  /**
   * @brief Thread does audio/video capture.
   */
  static inline constexpr char kCapture[8]{"Capture"};
  /**
   * @brief Thread does distribution?
   */
  static inline constexpr char kDistribution[13]{"Distribution"};
  /**
   * @brief Thread belongs to the game.
   */
  static inline constexpr char kGames[6]{"Games"};
  /**
   * @brief Thread does audio/video playback.
   */
  static inline constexpr char kPlayback[9]{"Playback"};
  /**
   * @brief Thread processes audio on a low level.
   */
  static inline constexpr char kProAudio[10]{"Pro Audio"};
  /**
   * @brief Thread belongs to Window Manager.
   */
  static inline constexpr char kWindowManager[15]{"Window Manager"};
};

/**
 * @brief MMCSS thread task.
 */
class ScopedMmcssThreadTask {
 public:
  /**
   * @brief Creates ScopedMmcssThreadTask
   * @param name Task name.
   * @return nothing.
   */
  explicit ScopedMmcssThreadTask(_In_z_ const char* name) noexcept
      : name_{name} {
    G3DCHECK(!!name);
  }

  /**
   * @brief Task name.
   * @return Task name.
   */
  const char* name() const noexcept { return name_; }

 private:
  /**
   * @brief Task name.
   */
  const char* name_;
};

/**
 * @brief The relative thread priority of this thread to other threads
 * performing a similar task.  See AVRT_PRIORITY.
 */
enum class ScopedMmcssThreadPriority {
  kVeryLow = -2,
  kLow = -1,
  kNormal = 0,
  kHigh = 1,
  kCritical = 2
};

/**
 * @brief Provides control over Multimedia Class Scheduler service (MMCSS)
 * thread settings.
 */
class WB_BASE_API ScopedMmcssThreadController {
 public:
  /**
   * @brief Creates ScopedMmcssThreadController
   * @param first_task The first task to be performed.  Task name must match the
   * name of one of the subkeys of the following key
   * HKLM\SOFTWARE\Microsoft\Windows
   * NT\CurrentVersion\Multimedia\SystemProfile\Tasks.
   * @param last_task The last task to be performed.  Task name must match the
   * name of one of the subkeys of the following key
   * HKLM\SOFTWARE\Microsoft\Windows
   * NT\CurrentVersion\Multimedia\SystemProfile\Tasks.
   * @return ScopedMmcssThreadController.
   */
  static std2::result<ScopedMmcssThreadController> New(
      const ScopedMmcssThreadTask& first_task,
      const ScopedMmcssThreadTask& last_task) noexcept;

  ScopedMmcssThreadController(ScopedMmcssThreadController&& s) noexcept;
  ScopedMmcssThreadController& operator=(ScopedMmcssThreadController&&) =
      delete;

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(ScopedMmcssThreadController);

  ~ScopedMmcssThreadController() noexcept;

  /**
   * @brief Retrieves the system responsiveness setting used by the multimedia
   * class scheduler service for current thread.
   * @return The system responsiveness value.  This value can range from 10 to
   * 100 percent.
   */
  std2::result<unsigned char> GetResponsivenessPercent() const noexcept;

  /**
   * @brief Adjusts the thread priority of the calling thread relative to other
   * threads performing the same task.
   */
  std::error_code SetPriority(ScopedMmcssThreadPriority priority) const noexcept;

 private:
  class ScopedMmcssThreadControllerImpl;

  WB_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()
    // Private member is not accessible to the DLL's client, including inline
    // functions.
    WB_MSVC_DISABLE_WARNING(4251)
    /**
     * @brief Actual implementation.
     */
    wb::base::un<ScopedMmcssThreadControllerImpl> impl_;
  WB_MSVC_END_WARNING_OVERRIDE_SCOPE()

  /**
   * @brief Creates ScopedMmcssThreadController.
   * @param first_task The first task to be performed.  Task name must match the
   * name of one of the subkeys of the following key
   * HKLM\SOFTWARE\Microsoft\Windows
   * NT\CurrentVersion\Multimedia\SystemProfile\Tasks.
   * @param last_task The last task to be performed.  Task name must match the
   * name of one of the subkeys of the following key
   * HKLM\SOFTWARE\Microsoft\Windows
   * NT\CurrentVersion\Multimedia\SystemProfile\Tasks.
   * @return nothing.
   */
  explicit ScopedMmcssThreadController(
      const ScopedMmcssThreadTask& first_task,
      const ScopedMmcssThreadTask& last_task) noexcept;

  /**
   * @brief Get initialization error code.
   * @return Error code.
   */
  [[nodiscard]] std::error_code error_code() const noexcept;
};
}  // namespace wb::base::windows::mmcss

#endif  // !WB_BASE_WIN_MMCSS_SCOPED_MMCSS_THREAD_CONTROLLER_H_
