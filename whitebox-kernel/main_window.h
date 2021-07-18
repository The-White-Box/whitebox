// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// App main window.

#ifndef WB_WHITEBOX_KERNEL_MAIN_WINDOW_H_
#define WB_WHITEBOX_KERNEL_MAIN_WINDOW_H_

#include <sal.h>
#include <tchar.h>

#include <cstddef>  // std::byte
#include <memory>

#include "base/base_macroses.h"
#include "base/sampling_profiler.h"
#include "base/windows/ui/accessibility_shortcut_keys_toggler.h"
#include "base/windows/ui/base_window.h"
#include "base/windows/ui/full_screen_window_toggler.h"
#include "base/windows/ui/keyboard.h"
#include "base/windows/ui/mouse.h"

/**
 * @brief Raw input handle.
 */
using HRAWINPUT = struct HRAWINPUT__ *;

namespace wb::kernel {
/**
 * @brief Main app window.
 */
class MainWindow : public wb::base::windows::ui::BaseWindow<MainWindow> {
  friend class BaseWindow<MainWindow>;

 public:
  WB_COMPILER_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()
    // C4355: We do not call this members here, just bind to it, so safe.
    WB_COMPILER_MSVC_DISABLE_WARNING(4355)
    // C4868: compiler may not enforce left-to-right evaluation order in braced
    // initializer list
    WB_COMPILER_MSVC_DISABLE_WARNING(4868)
    /**
     * @brief Main window ctor.
     * @param instance App instance.
     * @return nothing.
     */
    explicit MainWindow(_In_ HINSTANCE instance, _In_ int icon_id,
                        _In_ int icon_small_id) noexcept
        : BaseWindow{instance, icon_id, icon_small_id},
          mouse_{},
          keyboard_{},
          render_sampling_profiler_{
              wb::base::HighResolutionSamplingProfiler::clock::now()},
          full_screen_window_toggler_{},
          accessibility_shortcut_keys_toggler_{},
          is_window_active_{false} {}
  WB_COMPILER_MSVC_END_WARNING_OVERRIDE_SCOPE()

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(MainWindow);

  /**
   * @brief Move ctor.
   * @param w Window.
   * @return nothing.
   */
  MainWindow(MainWindow &&w) noexcept
      : BaseWindow{std::forward<MainWindow>(w)},
        mouse_{std::move(w.mouse_)},
        keyboard_{std::move(w.keyboard_)},
        render_sampling_profiler_{std::move(w.render_sampling_profiler_)},
        full_screen_window_toggler_{std::move(w.full_screen_window_toggler_)},
        accessibility_shortcut_keys_toggler_{
            std::move(w.accessibility_shortcut_keys_toggler_)},
        is_window_active_{std::move(w.is_window_active_)} {}
  /**
   * @brief Move window assigment.
   * @param w Window.
   * @return Reference to current window instance.
   */
  MainWindow &operator=(MainWindow &&w) noexcept {
    BaseWindow::operator=(std::forward<MainWindow>(w));
    std::swap(mouse_, w.mouse_);
    std::swap(keyboard_, w.keyboard_);
    std::swap(render_sampling_profiler_, w.render_sampling_profiler_);
    std::swap(full_screen_window_toggler_, w.full_screen_window_toggler_);
    std::swap(accessibility_shortcut_keys_toggler_,
              w.accessibility_shortcut_keys_toggler_);
    std::swap(is_window_active_, w.is_window_active_);
    return *this;
  }

 private:
  /**
   * @brief Mouse device.
   */
  wb::base::un<wb::base::windows::ui::Mouse> mouse_;
  /**
   * @brief Keyboard device.
   */
  wb::base::un<wb::base::windows::ui::Keyboard> keyboard_;
  /**
   * @brief Sampling profiler for rendering.
   */
  wb::base::HighResolutionSamplingProfiler render_sampling_profiler_;
  /**
   * @brief Scoped Full Screen toggler.
   */
  wb::base::un<wb::base::windows::ui::FullScreenWindowToggler>
      full_screen_window_toggler_;
  /**
   * @brief Windows Accessibility Shortcut Keys toggler.
   */
  wb::base::windows::ui::AccessibilityShortcutKeysToggler
      accessibility_shortcut_keys_toggler_;
  /**
   * @brief Is window active or not?
   */
  bool is_window_active_;
  [[maybe_unused]] std::byte pad_[sizeof(char *) - 1];

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
   * @brief Window input handler.
   * @param window Window.
   * @return The return value is the result of the message processing and
   * depends on the message.
   */
  LRESULT OnInput(_In_ HWND window, _In_ unsigned char input_code,
                  _In_ HRAWINPUT raw_input) noexcept;

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
}  // namespace wb::kernel

#endif  // !WB_WHITEBOX_KERNEL_MAIN_WINDOW_H_
