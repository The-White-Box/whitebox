// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.

#include "base/include/windows/compiler_config.h"
#include "base/include/windows/windows_light.h"

// #include "whitebox-config.h"

namespace {
LRESULT WindowMessageHandler(HWND h, UINT msg, WPARAM wp, LPARAM lp) {
  return DefWindowProc(h, msg, wp, lp);
}
}  // namespace

/**
 * @brief Windows app entry point.
 * @param instance App instance.
 * @param command_line Command line.
 * @param show_window_flags Show window flags.
 * @return App exit code.
 */
int WINAPI WinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE,
                   [[maybe_unused]] _In_ LPSTR command_line,
                   _In_ int show_window_flags) {
  WNDCLASSEX window_class = {sizeof(window_class)};
  window_class.lpfnWndProc = WindowMessageHandler;
  window_class.hInstance = instance;
  window_class.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
  window_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
  window_class.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_APPWORKSPACE);
  window_class.lpszClassName = L"WhiteBox_App_Half_Life2_Window_Class";
  RegisterClassEx(&window_class);

  HWND window{CreateWindowEx(0L,
                             window_class.lpszClassName, /* window class name*/
                             L"Half-Life 2 [64 bit]",    /* title  */
                             WS_OVERLAPPEDWINDOW,        /* style */
                             CW_USEDEFAULT, CW_USEDEFAULT, /* position */
                             CW_USEDEFAULT, CW_USEDEFAULT, /* size */
                             nullptr,                      /* parent */
                             nullptr,                      /* menu */
                             instance, 0)};
  ShowWindow(window, show_window_flags);

  while (true) {
    MSG msg = {0};
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT) return static_cast<int>(msg.wParam);

      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  return 0;
}