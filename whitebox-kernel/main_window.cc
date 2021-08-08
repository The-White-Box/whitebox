// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// App main window.

#include "main_window.h"

#include <timeapi.h>
#include <windowsx.h>

#include <chrono>

#include "base/deps/g3log/g3log.h"
#include "base/windows/ui/scoped_change_cursor.h"
#include "base/windows/ui/scoped_window_paint.h"
#include "base/windows/ui/task_dialog.h"
#include "base/windows/ui/window_message_handlers.h"
#include "base/windows/ui/window_utilities.h"
#include "base/windows/ui/windows_raw_input.h"
#include "build/static_settings_config.h"

namespace wb::kernel {
LRESULT MainWindow::HandleMessage(_In_ UINT message,
                                  _In_ [[maybe_unused]] WPARAM wParam,
                                  _In_ LPARAM lParam) noexcept {
  // Order desc by frequency.
  switch (message) {
    HANDLE_MSG(NativeHandle(), WM_INPUT, OnInput);
    HANDLE_MSG(NativeHandle(), WM_PAINT, OnPaint);
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

  using namespace wb::base::windows;

  // We are loading.
  ui::ScopedChangeCursor scoped_app_starting_cursor{
      ::LoadCursor(nullptr, IDC_APPSTARTING)};

  // For nice looking window.
  ui::MoveWindowToPrimaryDisplayCenter(window, false);

  // TODO(dimhotepus): What if only joystick?

  // TODO(dimhotepus): Localize.
  constexpr char kKernelDialogTitle[]{"Whitebox Kernel - Error"};
  constexpr char kSeeTechDetails[]{"See techical details"};
  constexpr char kHideTechDetails[]{"Hide techical details"};

  // Mouse is ready.
  mouse_.reset(new ui::Mouse{window});
  if (mouse_->error_code()) {
    ui::DialogBoxSettings dialog_settings(
        nullptr, kKernelDialogTitle, "Unable to initialize mouse device",
        "Unfortunately we unable to register mouse device for <A "
        "HREF=\"https://docs.microsoft.com/en-us/windows/win32/inputdev/"
        "about-raw-input\">Raw Input</A> data supply.  "
        "Please, contact authors.",
        kHideTechDetails, kSeeTechDetails, mouse_->error_code().message(),
        wb::build::settings::ui::error_dialog::kFooterLink,
        ui::DialogBoxButton::kOk, icon_id_, icon_small_id_, false);
    ui::ShowDialogBox(ui::DialogBoxKind::kError, dialog_settings);

    G3PLOG_E(FATAL, mouse_->error_code())
        << "Unable to initialize mouse device.";
  }

  // Keyboard is ready.
  keyboard_.reset(new ui::Keyboard{window});
  if (keyboard_->error_code()) {
    ui::DialogBoxSettings dialog_settings(
        nullptr, kKernelDialogTitle, "Unable to initialize keyboard device",
        "Unfortunately we unable to register keyboard device for <A "
        "HREF=\"https://docs.microsoft.com/en-us/windows/win32/inputdev/"
        "about-raw-input\">Raw Input</A> data supply.  "
        "Please, contact authors.",
        kHideTechDetails, kSeeTechDetails, mouse_->error_code().message(),
        wb::build::settings::ui::error_dialog::kFooterLink,
        ui::DialogBoxButton::kOk, icon_id_, icon_small_id_, false);
    ui::ShowDialogBox(ui::DialogBoxKind::kError, dialog_settings);

    G3PLOG_E(FATAL, keyboard_->error_code())
        << "Unable to initialize keyboard device.";
  }

  // Now can go full screen.
  full_screen_window_toggler_.reset(
      new ui::FullScreenWindowToggler{window, create_struct->style});

  using namespace std::chrono_literals;
  // TODO(dimhotepus): Simulate long loading.  Move to the task?
  std::this_thread::sleep_for(1s);

  return true;
}

LRESULT MainWindow::OnInput(_In_ HWND window, _In_ unsigned char input_code,
                            _In_ HRAWINPUT source_input) noexcept {
  G3DCHECK(input_code == RIM_INPUT || input_code == RIM_INPUTSINK);

  // If app is in foreground and query raw input data succeeded.
  if (input_code == RIM_INPUT) [[likely]] {
    using namespace wb::base::windows;

    RAWINPUT read_input;
    if (ui::ReadRawInput(source_input, read_input)) [[likely]] {
      bool is_raw_input_handled{false};

      if (mouse_ && keyboard_) [[likely]] {
        ui::MouseInput mouse_input;

        if (mouse_->Handle(read_input, mouse_input)) {
          // TODO(dimhotepus): Do smth with mouse.
          is_raw_input_handled = true;
        } else {
          ui::KeyboardInput keyboard_input;

          if (keyboard_->Handle(read_input, keyboard_input) &&
              keyboard_input.make_code != ui::KeyboardInput::kOverrunMakeCode) {
            // TODO(dimhotepus): Do smth with keyboard.
            is_raw_input_handled = true;

            if (keyboard_input.make_code == 0x57 &&
                (keyboard_input.key_flags & ui::KeyboardKeyFlags::kDown) ==
                    ui::KeyboardKeyFlags::kDown) {
              full_screen_window_toggler_->Toggle(
                  !full_screen_window_toggler_->IsFullScreen());
            }
          }
        }
      }

      // Nor mouse or keyboard so system can do what it needs with.
      if (!is_raw_input_handled) {
        const auto rc = ui::HandleNonHandledRawInput(sizeof(read_input.header));
        G3DCHECK(rc == 0);
      }
    }

    {
      // RIM_INPUT means "Input occurred while the application was in the
      // foreground.  The application must call DefWindowProc so the system can
      // perform cleanup."
      //
      // See
      // https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-input#parameters
      const auto rc =
          FORWARD_WM_INPUT(window, input_code, source_input, DefWindowProc);
      G3DCHECK(rc == 0);
    }

    return 0L;
  }

  // App doesn't handle RIM_INPUTSINK or other, signal system to handle it
  // itself.
  return 1L;
}

void MainWindow::OnPaint(_In_ HWND window) noexcept {
  G3DCHECK(!!window);

  render_sampling_profiler_.Sample();

  {
    wb::base::windows::ui::ScopedWindowPaint scoped_window_paint{window};

    using namespace std::chrono_literals;

    if (is_window_active_ && !::IsIconic(window)) {
      // TODO(dimhotepus): Repaint.

      {
        const auto get_fps_as_float = [](auto time_between_samples) noexcept {
          const auto elapsed_since_last_frame_mks =
              std::chrono::duration_cast<std::chrono::microseconds>(
                  time_between_samples)
                  .count();
          return elapsed_since_last_frame_mks
                     ? 1000000.0F /
                           static_cast<float>(elapsed_since_last_frame_mks)
                     : 0;
        };

        const float fps{get_fps_as_float(
            render_sampling_profiler_.GetTimeBetweenLastSamples())};

        char fps_text[128];
        sprintf_s(fps_text, "%.2f FPS", fps);

        {
          RECT paint_rc{scoped_window_paint.PaintInfo().rcPaint};
          const int w{paint_rc.right - paint_rc.left};
          const int h{paint_rc.bottom - paint_rc.top};

          scoped_window_paint.TextDraw(
              fps_text, -1, &paint_rc,
              DT_NOPREFIX | DT_VCENTER | DT_CENTER | DT_SINGLELINE);
        }
      }

      // Simulate render.
      std::this_thread::sleep_for(6ms);
    } else {
      // Inactive or iconic, do not draw too much system power.
      std::this_thread::sleep_for(30ms);
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
  min_track_size.x = build::settings::ui::window::dimensions::kMinWidth;
  min_track_size.y = build::settings::ui::window::dimensions::kMinHeight;
}

void MainWindow::OnWindowDestroy(_In_ HWND) noexcept { ::PostQuitMessage(0); }
}  // namespace wb::kernel
