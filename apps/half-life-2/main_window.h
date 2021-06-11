// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// App main window.

#ifndef WB_APPS_HALF_LIFE_2_MAIN_WINDOW_H_
#define WB_APPS_HALF_LIFE_2_MAIN_WINDOW_H_

#include <tchar.h>

#include <cstddef>  // std::byte

#include "base/include/base_macroses.h"
#include "base/include/sampling_profiler.h"
#include "base/include/windows/ui/base_window.h"

namespace wb::apps {
/**
 * @brief Main app window.
 */
class MainWindow : public wb::base::windows::ui::BaseWindow<MainWindow> {
  friend class BaseWindow<MainWindow>;

 public:
  /**
   * @brief Main window ctor.
   * @param instance App instance.
   * @return nothing.
   */
  MainWindow(_In_ HINSTANCE instance) noexcept
      : BaseWindow{instance},
        render_sampling_profiler_{
            wb::base::HighResolutionSamplingProfiler::clock::now()},
        is_window_active_{false} {}

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(MainWindow);

  /**
   * @brief Move ctor.
   * @param w Window.
   * @return nothing.
   */
  MainWindow(MainWindow &&w) noexcept
      : BaseWindow{std::forward<MainWindow>(w)},
        render_sampling_profiler_{std::move(w.render_sampling_profiler_)},
        is_window_active_{std::move(w.is_window_active_)} {}
  /**
   * @brief Move window assigment.
   * @param w Window.
   * @return Reference to current window instance.
   */
  MainWindow &operator=(MainWindow &&w) noexcept {
    BaseWindow::operator=(std::forward<MainWindow>(w));
    std::swap(is_window_active_, w.is_window_active_);
    std::swap(render_sampling_profiler_, w.render_sampling_profiler_);
    return *this;
  }

 private:
  /**
   * @brief Sampling profiler for rendering.
   */
  wb::base::HighResolutionSamplingProfiler render_sampling_profiler_;
  /**
   * @brief Is window active or not.
   */
  bool is_window_active_;
  [[maybe_unused]] std::byte pad_[sizeof(char *) - sizeof(is_window_active_)];

  /**
   * @brief Gets window class name.
   * @return Window class name.
   */
  [[nodiscard]] static PCTSTR ClassName() noexcept {
    return _T("WhiteBox_App_Half_Life2_Window_Class");
  }

  /**
   * @brief Window message handler.
   * @param message Message.
   * @param wParam Wide message parameter.
   * @param lParam Low message parameter.
   * @return Result code.
   */
  LRESULT HandleMessage(_In_ UINT message, _In_ [[maybe_unused]] WPARAM wParam,
                        _In_ LPARAM lParam) noexcept override;

  /**
   * @brief Window create handler.
   * @param window Window.
   * @param create_struct Window create data.
   * @return true when window can be created, false otherwise.
   */
  [[nodiscard]] bool OnWindowCreate(_In_ HWND window,
                                    _In_ CREATESTRUCT *create_struct) noexcept;

  /**
   * @brief Window paint handler.
   * @param window Window.
   * @return void.
   */
  void OnPaint(_In_ HWND window) noexcept;

  /**
   * @brief Window activate app handler.
   * @param  Window.
   * @param is_activating Is window activating or deactivating?
   * @param  Thread id.
   */
  void OnActivateApp(_In_ HWND, _In_ BOOL is_activating, _In_ DWORD) noexcept;

  /**
   * @brief Window resize bounds handler.
   * @param window.
   * @param min_max_info Min and max bounds info.
   * @return void.
   */
  void OnGetWindowSizeBounds(_In_ HWND, _In_ MINMAXINFO *min_max_info) noexcept;

  /**
   * @brief Window destroy handler.
   * @param window.
   * @return void.
   */
  void OnWindowDestroy(_In_ HWND) noexcept;
};
}  // namespace wb::apps

#endif  // !WB_APPS_HALF_LIFE_2_MAIN_WINDOW_H_
