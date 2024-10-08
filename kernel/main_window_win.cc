// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// App main window on Windows.

#include "main_window_win.h"

#include <timeapi.h>
#include <windowsx.h>

#include <chrono>

#include "base/deps/g3log/g3log.h"
#include "base/deps/mimalloc/mimalloc.h"
#include "base/intl/l18n.h"
#include "build/static_settings_config.h"
#include "hal/drivers/hid/raw_input_win.h"
#include "ui/fatal_dialog.h"
#include "ui/static_settings_config.h"
#include "ui/win/scoped_change_cursor.h"
#include "ui/win/scoped_window_paint.h"
#include "ui/win/window_message_handlers.h"
#include "ui/win/window_utilities.h"

namespace wb::kernel {

LRESULT MainWindow::HandleMessage(_In_ UINT message,
                                  _In_ [[maybe_unused]] WPARAM wParam,
                                  _In_ LPARAM lParam) noexcept {
  // Descending order by usage frequency.
  switch (message) {
    HANDLE_MSG(NativeHandle(), WM_PAINT, OnPaint);
    HANDLE_MSG(NativeHandle(), WM_INPUT, OnInput);
    HANDLE_MSG(NativeHandle(), WM_ACTIVATEAPP, OnActivateApp);
    HANDLE_MSG(NativeHandle(), WM_GETMINMAXINFO, OnGetWindowSizeBounds);
    HANDLE_MSG(NativeHandle(), WM_CREATE, OnWindowCreate);
    HANDLE_MSG(NativeHandle(), WM_DESTROY, OnWindowDestroy);
  }
  return DefWindowProc(NativeHandle(), message, wParam, lParam);
}

[[nodiscard]] bool MainWindow::OnWindowCreate(
    _In_ HWND window, _In_ CREATESTRUCT *create_struct) noexcept {
  G3DCHECK(!!window);

  using namespace wb::base;
  using namespace wb::base::win;
  using namespace wb::ui::win;

  // We are loading.
  ScopedChangeCursor scoped_app_starting_cursor{
      ::LoadCursor(nullptr, IDC_APPSTARTING)};

  // For nice looking window.
  MoveWindowToItsDisplayCenter(window, false);

  {
    // Mouse is ready.
    auto mouse_result = hal::hid::Mouse::New(window);
    if (mouse_result.has_value()) [[likely]] {
      mouse_.swap(*mouse_result);
    } else {
      return !ui::FatalDialog(
          intl::l18n(intl_, "Whitebox Kernel - Error"), mouse_result.error(),
          intl::l18n(intl_, "Please, check mouse is connected and working."),
          MakeFatalContext(),
          intl::l18n(
              intl_,
              "Unable to register mouse as <A "
              "HREF=\"https://docs.microsoft.com/en-us/windows/win32/inputdev/"
              "about-raw-input\">Raw Input</A> device."));
    }
  }

  {
    // Keyboard is ready.
    auto keyboard_result = hal::hid::Keyboard::New(window);
    if (keyboard_result.has_value()) [[likely]] {
      keyboard_.swap(*keyboard_result);
    } else {
      return !ui::FatalDialog(
          intl::l18n(intl_, "Whitebox Kernel - Error"), keyboard_result.error(),
          intl::l18n(intl_, "Please, check keyboard is connected and working."),
          MakeFatalContext(),
          intl::l18n(
              intl_,
              "Unable to register keyboard as <A "
              "HREF=\"https://docs.microsoft.com/en-us/windows/win32/inputdev/"
              "about-raw-input\">Raw Input</A> device."));
    }
  }

  {
    auto full_screen_toggler =
        std::make_unique<FullScreenWindowToggler>(window, create_struct->style);
    // Now can go full screen.
    full_screen_window_toggler_.swap(full_screen_toggler);

    // When window is of normal size, should enable DWM MMCSS to speed up window
    // composition.
    ToggleDwmMmcss(!full_screen_window_toggler_->IsFullScreen());
  }

  return true;
}

LRESULT MainWindow::OnInput(_In_ HWND window, _In_ unsigned char input_code,
                            _In_ HRAWINPUT source_input) noexcept {
  G3DCHECK(input_code == RIM_INPUT || input_code == RIM_INPUTSINK);

  // App doesn't handle RIM_INPUTSINK or other, signal system to handle it
  // itself.
  if (input_code != RIM_INPUT) [[unlikely]] {
    return 1L;
  }

  // App is in foreground and query raw input data succeeded.
  using namespace wb::base;
  using namespace wb::base::win;

  const auto time = HighResolutionClock::now();

  RAWINPUT read_input;
  if (hal::hid::ReadRawInput(source_input, read_input)) [[likely]] {
    bool is_raw_input_handled{false};

    G3DCHECK(!!mouse_);
    G3DCHECK(!!keyboard_);

    hal::hid::MouseInput mouse_input;

    if (mouse_->Handle(read_input, mouse_input)) {
      is_raw_input_handled = true;

      mouse_input_queue_.Emplace(time, mouse_input);
    } else {
      hal::hid::KeyboardInput keyboard_input;

      if (keyboard_->Handle(read_input, keyboard_input) &&
          keyboard_input.make_code !=
              hal::hid::KeyboardInput::kOverrunMakeCode) {
        is_raw_input_handled = true;

        keyboard_input_queue_.Emplace(time, keyboard_input);

        if (keyboard_input.make_code == 0x57 &&
            (keyboard_input.key_flags & hal::hid::KeyboardKeyFlags::kDown) ==
                hal::hid::KeyboardKeyFlags::kDown) {
          const bool need_full_screen{
              !full_screen_window_toggler_->IsFullScreen()};

          full_screen_window_toggler_->Toggle(need_full_screen);

          ToggleDwmMmcss(!need_full_screen);
        }
      }
    }

    // Nor mouse or keyboard so system can do what it needs with.
    if (!is_raw_input_handled) [[unlikely]] {
      const auto rc =
          hal::hid::HandleNonHandledRawInput(sizeof(read_input.header));
      G3DCHECK(rc == 0);
    }
  }

  // RIM_INPUT means "Input occurred while the application was in the
  // foreground.  The application must call DefWindowProc so the system
  // can perform cleanup."
  //
  // See
  // https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-input#parameters
  const auto rc =
      FORWARD_WM_INPUT(window, input_code, source_input, DefWindowProc);
  G3DCHECK(rc == 0);

  return rc;
}

void MainWindow::OnPaint(_In_ HWND window) noexcept {
  G3DCHECK(!!window);

  using namespace wb::base;
  using namespace std::chrono_literals;

  render_sampling_profiler_.Sample();

  if (!is_window_active_ || ::IsIconic(window)) [[unlikely]] {
    // Inactive or iconic, do not draw too much system power.
    std::this_thread::sleep_for(30ms);
  }

  // Simulate render.
  {
    const auto get_fps = [](auto delta) noexcept {
      using namespace std::chrono;

      using duration_ms = microseconds;

      const auto elapsed_ms = duration_cast<duration_ms>(delta).count();
      return elapsed_ms
                 ? (duration_ms::period::den / duration_ms::period::num) /
                       static_cast<float>(elapsed_ms)
                 : 0;
    };

    const float fps{
        get_fps(render_sampling_profiler_.GetTimeBetweenLastSamples())};
    constexpr float kFpsMaxCap{200};

    if (fps <= kFpsMaxCap) {
      auto scoped_window_paint = ui::win::ScopedWindowPaint::New(window);

      scoped_window_paint.transform(
          [=](const ui::win::ScopedWindowPaint &painter) {
            RECT paint_rc{painter.PaintInfo().rcPaint};

            if (!::IsRectEmpty(&paint_rc)) {
              std::string message;
              absl::StrAppend(&message,
                              "FPS: ", std::floor(fps * 10.0F) / 10.0F);

              painter.BlitPattern(paint_rc, WHITENESS);
              painter.TextDraw(
                  message.c_str(), -1, &paint_rc,
                  DT_NOPREFIX | DT_VCENTER | DT_CENTER | DT_SINGLELINE);
            }
          });
    } else {
      std::this_thread::sleep_for(4ms);
    }
  }

  // Generate continuous stream of WM_PAINT to render with up to display
  // update speed.
  ::InvalidateRect(window, nullptr, FALSE);
}

void MainWindow::OnActivateApp(_In_ HWND, _In_ BOOL is_activating,
                               _In_ DWORD) noexcept {
  const bool is_active{is_activating == TRUE};

  is_window_active_ = is_active;
  // Disable accessibility shortcut keys when window is active.
  accessibility_shortcut_keys_toggler_.Toggle(!is_active);
}

void MainWindow::OnGetWindowSizeBounds(_In_ HWND,
                                       _In_ MINMAXINFO *min_max_info) noexcept {
  G3DCHECK(!!min_max_info);

  auto &min_track_size = min_max_info->ptMinTrackSize;
  min_track_size.x = ui::settings::window::dimensions::kMinWidth;
  min_track_size.y = ui::settings::window::dimensions::kMinHeight;
}

void MainWindow::OnWindowDestroy(_In_ HWND) noexcept { ::PostQuitMessage(0); }

void MainWindow::ToggleDwmMmcss(_In_ bool enable) noexcept {
  using namespace wb::base;

  if (enable) {
    // Change window to normal size, should enable DWM MMCSS to speed up window
    // composition.
    auto scoped_toggle_dwm_mmcs_result =
        win::mmcss::ScopedMmcssToggleDwm::New(true);
    if (scoped_toggle_dwm_mmcs_result.has_value()) {
      auto new_scheduler = std::optional<win::mmcss::ScopedMmcssToggleDwm>{
          std::move(*scoped_toggle_dwm_mmcs_result)};
      scoped_mmcss_toggle_dwm_.swap(new_scheduler);
    } else {
      G3PLOG_E(WARNING, scoped_toggle_dwm_mmcs_result.error())
          << "Unable to enable Desktop Window Manager (DWM) Multimedia Class "
             "Schedule Service (MMCSS) scheduling.  Rendering will not be "
             "optimized for multimedia applications.";
    }
  } else {
    // Full screen, disable DWM MMCSS.
    scoped_mmcss_toggle_dwm_.reset();
  }
}

[[nodiscard]] ui::FatalDialogContext MainWindow::MakeFatalContext() noexcept {
  return {intl_, intl_.Layout(), icon_id_, icon_small_id_};
}

}  // namespace wb::kernel
