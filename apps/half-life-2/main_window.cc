// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// App main window.

#include "main_window.h"

#include <tchar.h>
#include <timeapi.h>
#include <windowsx.h>

#include <chrono>

#include "base/include/windows/ui/scoped_change_cursor.h"
#include "base/include/windows/ui/scoped_window_paint.h"
#include "base/include/windows/ui/window_utilities.h"

namespace whitebox::apps {
LRESULT MainWindow::HandleMessage(_In_ UINT message,
                                  _In_ [[maybe_unused]] WPARAM wParam,
                                  _In_ LPARAM lParam) noexcept {
  // Order desc by frequency.
  switch (message) {
    HANDLE_MSG(NativeHandle(), WM_PAINT, OnPaint);
    HANDLE_MSG(NativeHandle(), WM_ACTIVATEAPP, OnActivateApp);
    HANDLE_MSG(NativeHandle(), WM_GETMINMAXINFO, OnGetWindowSizeBounds);
    HANDLE_MSG(NativeHandle(), WM_CREATE, OnWindowCreate);
    HANDLE_MSG(NativeHandle(), WM_DESTROY, OnWindowDestroy);
  }
  return DefWindowProc(NativeHandle(), message, wParam, lParam);
}

[[nodiscard]] bool MainWindow::OnWindowCreate(_In_ HWND window,
                                              _In_ CREATESTRUCT *) noexcept {
  DCHECK(!!window);

  using namespace whitebox::base::windows;

  ui::ScopedChangeCursor scoped_app_starting_cursor{
      ::LoadCursor(nullptr, IDC_APPSTARTING)};

  ui::MoveWindowToPrimaryDisplayCenter(window, false);

  using namespace std::chrono_literals;
  // TODO(dimhotepus): Simulate long loading.  Move to task?
  std::this_thread::sleep_for(1s);

  return true;
}

void MainWindow::OnPaint(_In_ HWND window) noexcept {
  DCHECK(!!window);

  render_sampling_profiler_.Sample();

  {
    whitebox::base::windows::ui::ScopedWindowPaint scoped_window_paint{window};

    using namespace std::chrono_literals;

    if (is_window_active_ && !::IsIconic(window)) {
      // TODO(dimhotepus): Repaint.

      // Simulate render.
      std::this_thread::sleep_for(8ms);
    } else {
      // Inactive or iconic, do not draw too much system power.
      std::this_thread::sleep_for(30ms);
    }
  }

  {
    const auto elapsed_since_last_frame_mks =
        std::chrono::duration_cast<std::chrono::microseconds>(
            render_sampling_profiler_.GetTimeBetweenLastSamples())
            .count();
    const float fps{elapsed_since_last_frame_mks
                        ? 1000000.0F /
                              static_cast<float>(elapsed_since_last_frame_mks)
                        : 0};

    TCHAR window_title[128];
    _stprintf_s(window_title, _T("Half-Life 2 [64 bit] - %.2f FPS"), fps);

    ::SetWindowText(window, window_title);
  }

  // Generate continuous stream of WM_PAINT to render with up to display update
  // speed.
  ::InvalidateRect(window, nullptr, FALSE);
}

void MainWindow::OnActivateApp(_In_ HWND, _In_ BOOL is_activating,
                               _In_ DWORD) noexcept {
  is_window_active_ = is_activating == TRUE;
}

void MainWindow::OnGetWindowSizeBounds(_In_ HWND,
                                       _In_ MINMAXINFO *min_max_info) noexcept {
  DCHECK(!!min_max_info);

  auto &min_track_size = min_max_info->ptMinTrackSize;
  min_track_size.x = 640;
  min_track_size.y = 480;
}

void MainWindow::OnWindowDestroy(_In_ HWND) noexcept { PostQuitMessage(0); }
}  // namespace whitebox::apps
