// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// App main window on Windows.

#ifndef WB_KERNEL_MAIN_WINDOW_WIN_H_
#define WB_KERNEL_MAIN_WINDOW_WIN_H_

#include <sal.h>

#include <array>
#include <cstddef>  // std::byte
#include <memory>
#include <optional>

#include "base/intl/lookup.h"
#include "base/macroses.h"
#include "base/sampling_profiler.h"
#include "base/win/mmcss/scoped_mmcss_toggle_dwm.h"
#include "hal/drivers/hid/keyboard_win.h"
#include "hal/drivers/hid/mouse_win.h"
#include "kernel/input/input_queue.h"
#include "ui/fatal_dialog.h"
#include "ui/win/accessibility_shortcut_keys_toggler.h"
#include "ui/win/base_window.h"
#include "ui/win/full_screen_window_toggler.h"

/**
 * @brief Raw input handle.
 */
using HRAWINPUT = struct HRAWINPUT__ *;

namespace wb::kernel {

/**
 * @brief Main app window.
 */
class MainWindow : public wb::ui::win::BaseWindow {
  friend class BaseWindow;

 public:
  WB_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()
    // C4355: We do not call this members here, just bind to it, so safe.
    WB_MSVC_DISABLE_WARNING(4355)
    // C4868: compiler may not enforce left-to-right evaluation order in braced
    // initializer list
    WB_MSVC_DISABLE_WARNING(4868)
    /**
     * @brief Main window ctor.
     * @param instance App instance.
     * @return nothing.
     */
    MainWindow(_In_ HINSTANCE instance, int icon_id, int icon_small_id,
               const wb::base::intl::LookupWithFallback &l18n,
               input::InputQueue<hal::hid::MouseInput> &mouse_input_queue,
               input::InputQueue<hal::hid::KeyboardInput>
                   &keyboard_input_queue) noexcept
        : BaseWindow{instance, icon_id, icon_small_id},
          mouse_{},
          mouse_input_queue_{mouse_input_queue},
          keyboard_{},
          keyboard_input_queue_{keyboard_input_queue},
          render_sampling_profiler_{
              wb::base::HighResolutionSamplingProfiler::clock::now()},
          full_screen_window_toggler_{},
          accessibility_shortcut_keys_toggler_{},
          scoped_mmcss_toggle_dwm_{},
          intl_{l18n},
          is_window_active_{false} {}
  WB_MSVC_END_WARNING_OVERRIDE_SCOPE()

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(MainWindow);

  /**
   * @brief Move ctor.
   * @param w Window.
   * @return nothing.
   */
  MainWindow(MainWindow &&w) noexcept
      : BaseWindow{std::forward<MainWindow>(w)},
        mouse_{std::move(w.mouse_)},
        mouse_input_queue_{w.mouse_input_queue_},
        keyboard_{std::move(w.keyboard_)},
        keyboard_input_queue_{w.keyboard_input_queue_},
        render_sampling_profiler_{std::move(w.render_sampling_profiler_)},
        full_screen_window_toggler_{std::move(w.full_screen_window_toggler_)},
        accessibility_shortcut_keys_toggler_{
            std::move(w.accessibility_shortcut_keys_toggler_)},
        scoped_mmcss_toggle_dwm_{std::move(w.scoped_mmcss_toggle_dwm_)},
        intl_{w.intl_},
        is_window_active_{std::move(w.is_window_active_)} {}
  /**
   * @brief Move window assigment.
   * @param w Window.
   * @return Reference to current window instance.
   */
  MainWindow &operator=(MainWindow &&w) noexcept {
    BaseWindow::operator=(std::forward<MainWindow>(w));
    std::swap(mouse_, w.mouse_);
    std::swap(mouse_input_queue_, w.mouse_input_queue_);
    std::swap(keyboard_, w.keyboard_);
    std::swap(keyboard_input_queue_, w.keyboard_input_queue_);
    std::swap(render_sampling_profiler_, w.render_sampling_profiler_);
    std::swap(full_screen_window_toggler_, w.full_screen_window_toggler_);
    std::swap(accessibility_shortcut_keys_toggler_,
              w.accessibility_shortcut_keys_toggler_);
    scoped_mmcss_toggle_dwm_.swap(w.scoped_mmcss_toggle_dwm_);
    std::swap(is_window_active_, w.is_window_active_);
    return *this;
  }

  /**
   * @brief Gets window class name.
   * @return Window class name.
   */
  [[nodiscard]] static std::string ClassName(std::string_view window_name) noexcept {
    return "WhiteBox " + std::string{window_name} + " Window Class";
  }

 private:
  /**
   * @brief Mouse device.
   */
  base::un<hal::hid::Mouse> mouse_;
  /**
   * @brief Mouse input queue.
   */
  input::InputQueue<hal::hid::MouseInput> &mouse_input_queue_;
  /**
   * @brief Keyboard device.
   */
  base::un<hal::hid::Keyboard> keyboard_;
  /**
   * @brief Keyboard input queue.
   */
  input::InputQueue<hal::hid::KeyboardInput> &keyboard_input_queue_;
  /**
   * @brief Sampling profiler for rendering.
   */
  base::HighResolutionSamplingProfiler render_sampling_profiler_;
  /**
   * @brief Scoped Full Screen toggler.
   */
  base::un<ui::win::FullScreenWindowToggler> full_screen_window_toggler_;
  /**
   * @brief Windows Accessibility Shortcut Keys toggler.
   */
  ui::win::AccessibilityShortcutKeysToggler
      accessibility_shortcut_keys_toggler_;
  /**
   * @brief Display Window Manager runs using Multimedia Class Schedule Service
   * (MMCSS) scheduling to speed up window composition.
   */
  std::optional<base::win::mmcss::ScopedMmcssToggleDwm>
      scoped_mmcss_toggle_dwm_;
  /**
   * @brief Localization service.
   */
  const base::intl::LookupWithFallback &intl_;
  /**
   * @brief Is window active or not?
   */
  bool is_window_active_;

  [[maybe_unused]] std::array<std::byte,
                              sizeof(char *) - sizeof(is_window_active_)>
      pad_;

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

 private:
  /**
   * @brief When window is of normal size, should enable DWM MMCSS to speed up
   * window composition.
   * @param enable Enable DWM MMCSS?
   * @return void.
   */
  void ToggleDwmMmcss(_In_ bool enable) noexcept;

  /**
   * @brief Create fatal dialog context.
   * @return Fatal dialog context.
   */
  [[nodiscard]] ui::FatalDialogContext MakeFatalContext() noexcept;
};

}  // namespace wb::kernel

#endif  // !WB_KERNEL_MAIN_WINDOW_WIN_H_
