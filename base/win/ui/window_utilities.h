// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Common window utilities.

#ifndef WB_BASE_WIN_UI_WINDOW_UTILITIES_H_
#define WB_BASE_WIN_UI_WINDOW_UTILITIES_H_

#include <sal.h>

#include "base/base_api.h"

using HWND = struct HWND__ *;

namespace wb::base::windows::ui {
/**
 * @brief Moves window to the center of its monitor.
 * @param hwnd Window.
 * @param repaint_after Should repaint window after move?
 * @return true on success, false otherwise.
 */
WB_BASE_API bool MoveWindowToItsDisplayCenter(_In_ HWND window,
                                              _In_ bool repaint_after) noexcept;
}  // namespace wb::base::windows::ui

#endif  // !WB_BASE_WIN_UI_WINDOW_UTILITIES_H_
