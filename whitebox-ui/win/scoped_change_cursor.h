// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Scoped cursor changer.

#ifndef WB_WHITEBOX_UI_WIN_SCOPED_CHANGE_CURSOR_H_
#define WB_WHITEBOX_UI_WIN_SCOPED_CHANGE_CURSOR_H_

#include "base/macroses.h"
#include "build/compiler_config.h"

using HICON = struct HICON__*;
using HCURSOR = HICON; /* HICONs & HCURSORs are polymorphic */

WB_ATTRIBUTE_DLL_IMPORT HCURSOR __stdcall SetCursor(_In_opt_ HCURSOR hCursor);

namespace wb::ui::win {

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

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedChangeCursor);

  /**
   * @brief Restore previous cursor.
   */
  ~ScopedChangeCursor() noexcept { ::SetCursor(old_cursor_); }

 private:
  /**
   * @brief Previous cursor.
   */
  _In_opt_ const HCURSOR old_cursor_;
};

}  // namespace wb::ui::win

#endif  // !WB_WHITEBOX_UI_WIN_SCOPED_CHANGE_CURSOR_H_
