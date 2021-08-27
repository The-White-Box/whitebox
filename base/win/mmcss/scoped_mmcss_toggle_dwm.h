// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Notifies the Desktop Window Manager (DWM) to opt in to or out of Multimedia
// Class Schedule Service(MMCSS) scheduling while the calling process is alive.

#ifndef WB_BASE_WIN_MMCSS_SCOPED_MMCSS_TOGGLE_DWM_H_
#define WB_BASE_WIN_MMCSS_SCOPED_MMCSS_TOGGLE_DWM_H_

#include <cstddef>  // std::byte

#include "base/base_macroses.h"
#include "base/deps/g3log/g3log.h"
#include "base/win/system_error_ext.h"

using HRESULT = long;

extern "C" WB_ATTRIBUTE_DLL_IMPORT HRESULT __stdcall DwmEnableMMCSS(
    _In_ int fEnableMMCSS);

namespace wb::base::windows::mmcss {
/**
 * @brief Notifies the Desktop Window Manager (DWM) to opt in to or out of
 * Multimedia Class Schedule Service (MMCSS) scheduling while the calling
 * process is alive.  Use this for non fullscreen exclusive applications and
 * games, as it should theoretically give process a boost in a responsiveness
 * from both DWM (graphics scheduling) and CSRSS (mouse and keyboard raw input).
 */
class ScopedMmcssToggleDwm {
 public:
  /**
   * @brief Create ScopedToggleDwmMultimediaClassScheduler with toggle on/off
   * state.
   * @param enable Enable MMCSS for DWM.
   * @return ScopedToggleDwmMultimediaClassScheduler.
   */
  static std_ext::os_res<ScopedMmcssToggleDwm> New(_In_ bool enable) noexcept {
    ScopedMmcssToggleDwm scheduler{enable};
    return !scheduler.error_code()
               ? std_ext::os_res<ScopedMmcssToggleDwm>{std::move(scheduler)}
               : std_ext::os_res<ScopedMmcssToggleDwm>{scheduler.error_code()};
  }

  ScopedMmcssToggleDwm(ScopedMmcssToggleDwm&& s) noexcept
      : error_code_{std::move(s.error_code_)},
        is_dwm_mmcss_enabled_{std::move(s.is_dwm_mmcss_enabled_)} {
    s.error_code_ = std::error_code{EINVAL, std::system_category()};
  }
  ScopedMmcssToggleDwm& operator=(ScopedMmcssToggleDwm&&) = delete;

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(ScopedMmcssToggleDwm);

  ~ScopedMmcssToggleDwm() noexcept {
    if (!error_code()) {
      G3CHECK(!GetErrorCode(
          ::DwmEnableMMCSS(is_dwm_mmcss_enabled_ ? FALSE : TRUE)));
    }
  }

 private:
  /**
   * @brief DWM MMCSS toggle error code.
   */
  std::error_code error_code_;
  /**
   * @brief Is DWM MMCSS enabled?
   */
  bool is_dwm_mmcss_enabled_;

  [[maybe_unused]] std::byte
      pad_[sizeof(char*) - sizeof(is_dwm_mmcss_enabled_)];

  /**
   * @brief Creates ScopedToggleDwmMultimediaClassScheduler
   * @param enable Enable MMCSS for DWM.
   * @return nothing.
   */
  explicit ScopedMmcssToggleDwm(_In_ bool enable) noexcept
      : error_code_{GetErrorCode(::DwmEnableMMCSS(enable ? TRUE : FALSE))},
        is_dwm_mmcss_enabled_{enable} {}

  /**
   * @brief Get initialization error code.
   * @return Error code.
   */
  [[nodiscard]] std::error_code error_code() const noexcept {
    return error_code_;
  }
};
}  // namespace wb::base::windows::mmcss

#endif  // !WB_BASE_WIN_MMCSS_SCOPED_MMCSS_TOGGLE_DWM_H_