// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Window message handlers.

#ifndef WB_WHITEBOX_UI_WIN_WINDOW_MESSAGE_HANDLERS_H_
#define WB_WHITEBOX_UI_WIN_WINDOW_MESSAGE_HANDLERS_H_

#include <windowsx.h>

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

// LRESULT Cls_OnInput(HWND hwnd, unsigned char input_code, HRAWINPUT input)
#define HANDLE_WM_INPUT(hwnd, wParam, lParam, fn) \
  ((fn)((hwnd), GET_RAWINPUT_CODE_WPARAM(wParam), (HRAWINPUT)(lParam)))
#define FORWARD_WM_INPUT(hwnd, input_code, input, fn)                  \
  (LRESULT)(fn)((hwnd), WM_INPUT, (WPARAM)(unsigned char)(input_code), \
                (LPARAM)(HRAWINPUT)(input))

#endif  // !WB_WHITEBOX_UI_WIN_WINDOW_MESSAGE_HANDLERS_H_
