// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Window message handlers.

#ifndef WB_BASE_WINDOWS_UI_WINDOW_MESSAGE_HANDLERS_H_
#define WB_BASE_WINDOWS_UI_WINDOW_MESSAGE_HANDLERS_H_

#include <windowsx.h>

#include "base/windows/windows_light.h"

#define HANDLE_TD_MSG(hwnd, message, fn) \
  case (message):                        \
    return HANDLE_##message((hwnd), (wParam), (lParam), (lpRefData), (fn))

/* void Cls_OnTaskDialogConstructed(HWND dialog, LONG_PTR ref_data) */
#define HANDLE_TDN_DIALOG_CONSTRUCTED(hwnd, wParam, lParam, lpRefData, fn) \
  ((fn)((hwnd), (lpRefData)), S_OK)

/* void Cls_OnTaskDialogHyperlinkClicked(HWND dialog, LONG_PTR ref_data, const
   wchar_t *url) */
#define HANDLE_TDN_HYPERLINK_CLICKED(hwnd, wParam, lParam, lpRefData, fn) \
  ((fn)((hwnd), (lpRefData), (LPCWSTR)(LPARAM)(lParam)), S_OK)

#undef HANDLE_WM_COPYDATA
/* BOOL Cls_OnCopyData(HWND hwnd, COPYDATASTRUCT* copy_data_struct) */
#define HANDLE_WM_COPYDATA(hwnd, wParam, lParam, fn) \
  ((fn)((hwnd), (COPYDATASTRUCT *)(lParam))) ? TRUE : FALSE

#undef FORWARD_WM_COPYDATA
#define FORWARD_WM_COPYDATA(hwnd, copy_data_struct, fn)        \
  (BOOL)(UINT)(DWORD)(fn)((hwnd), WM_COPYDATA, (WPARAM)(hwnd), \
                          (LPARAM)(LONG_PTR)(COPYDATASTRUCT *)(lParam))

// LRESULT Cls_OnImeNotify(HWND hwnd, int ime_command, LPARAM command_data)
#define HANDLE_WM_IME_NOTIFY(hwnd, wParam, lParam, fn) \
  ((fn)((hwnd), (int)(wParam), (lParam)))
#define FORWARD_WM_IME_NOTIFY(hwnd, ime_command, command_data)     \
  (LRESULT)(fn)((hwnd), WM_IME_NOTIFY, (WPARAM)(int)(ime_command), \
                (LPARAM)(command_data))

// LRESULT Cls_OnInput(HWND hwnd, unsigned char input_code, HRAWINPUT input)
#define HANDLE_WM_INPUT(hwnd, wParam, lParam, fn) \
  ((fn)((hwnd), GET_RAWINPUT_CODE_WPARAM(wParam), (HRAWINPUT)(lParam)))
#define FORWARD_WM_INPUT(hwnd, input_code, input, fn)                  \
  (LRESULT)(fn)((hwnd), WM_INPUT, (WPARAM)(unsigned char)(input_code), \
                (LPARAM)(HRAWINPUT)(input))

#undef HANDLE_WM_SYSCOMMAND
// LRESULT Cls_OnSysCommand(HWND hwnd, UINT cmd, int x, int y)
#define HANDLE_WM_SYSCOMMAND(hwnd, wParam, lParam, fn)              \
  ((fn)((hwnd), (unsigned int)(wParam), (int)(short)LOWORD(lParam), \
        (int)(short)HIWORD(lParam)))

#undef FORWARD_WM_SYSCOMMAND
#define FORWARD_WM_SYSCOMMAND(hwnd, cmd, x, y, fn)                  \
  (LRESULT)(fn)((hwnd), WM_SYSCOMMAND, (WPARAM)(unsigned int)(cmd), \
                MAKELPARAM((x), (y)))

#endif  // !WB_BASE_WINDOWS_UI_WINDOW_MESSAGE_HANDLERS_H_
