// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Scoped cursor changer.

#ifndef WHITEBOX_BASE_INCLUDE_WINDOWS_UI_SCOPED_CHANGE_CURSOR_H_
#define WHITEBOX_BASE_INCLUDE_WINDOWS_UI_SCOPED_CHANGE_CURSOR_H_
#ifdef _WIN32
#pragma once
#endif

#include "base/include/base_macroses.h"

#ifndef _MAC
using HICON = struct HICON__*;
using HCURSOR = HICON; /* HICONs & HCURSORs are polymorphic */
#else
using HCURSOR = struct HCURSOR__*; /* HICONs & HCURSORs are not polymorphic */
#endif

__declspec(dllimport) HCURSOR __stdcall SetCursor(_In_opt_ HCURSOR hCursor);

namespace whitebox::base::windows::ui {
/**
 * @brief Changes cursor in scope and reverts back when out of scope.
 */
class ScopedChangeCursor {
 public:
  /**
   * @brief Creates scoped cursor and set it.
   * @param new_cursor Cursor to apply in scope.
   * @return nothing.
   */
  explicit ScopedChangeCursor(_In_opt_ HCURSOR new_cursor) noexcept
      : old_cursor_{::SetCursor(new_cursor)} {}

  WHITEBOX_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedChangeCursor);

  ~ScopedChangeCursor() noexcept { (void)::SetCursor(old_cursor_); }

 private:
  _In_opt_ const HCURSOR old_cursor_;
};
}  // namespace whitebox::base::windows::ui

#endif  // !WHITEBOX_BASE_INCLUDE_WINDOWS_UI_SCOPED_CHANGE_CURSOR_H_
