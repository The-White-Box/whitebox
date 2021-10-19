// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Base UI window.

#ifndef WB_WHITEBOX_UI_WIN_WINDOW_DEFINITION_H_
#define WB_WHITEBOX_UI_WIN_WINDOW_DEFINITION_H_

#include <sal.h>

using HBRUSH = struct HBRUSH__ *;
using HINSTANCE = struct HINSTANCE__ *;
using HICON = struct HICON__ *;
using HCURSOR = HICON; /* HICONs & HCURSORs are polymorphic */
using HMENU = struct HMENU__ *;
using HWND = struct HWND__ *;
using DWORD = unsigned long;

namespace wb::ui::win {

/**
 * @brief Use default position for window.
*/
constexpr inline int kUseDefaultPosition{(int)0x80000000};

/**
 * @brief Window definition.
 */
struct WindowDefinition {
  /**
   * @brief Creates window definition.
   * @param instance_
   * @param name_
   * @param icon_id_
   * @param icon_small_id_
   * @param cursor_
   * @param class_brush_
   * @param style_
   * @param ex_style_
   * @param x_pos_
   * @param y_pos_
   * @param width_
   * @param height_
   * @param parent_window_
   * @param menu_
   */
  WindowDefinition(_In_opt_ HINSTANCE instance_, _In_opt_ const char *name_,
                   _In_ int icon_id_, _In_ int icon_small_id_,
                   _In_opt_ HCURSOR cursor_, _In_opt_ HBRUSH class_brush_,
                   _In_ unsigned long style_, _In_ unsigned long ex_style_ = 0,
                   _In_ int x_pos_ = kUseDefaultPosition,
                   _In_ int y_pos_ = kUseDefaultPosition,
                   _In_ int width_ = kUseDefaultPosition,
                   _In_ int height_ = kUseDefaultPosition,
                   _In_opt_ HWND parent_window_ = nullptr,
                   _In_opt_ HMENU menu_ = nullptr) noexcept
      : instance{instance_},
        name{name_},
        icon_id{icon_id_},
        icon_small_id{icon_small_id_},
        cursor{cursor_},
        class_brush{class_brush_},
        style{style_},
        ex_style{ex_style_},
        x_pos{x_pos_},
        y_pos{y_pos_},
        width{width_},
        height{height_},
        parent_window{parent_window_},
        menu{menu_} {}

  HINSTANCE instance;
  const char *name;
  int icon_id, icon_small_id;
  HCURSOR cursor;
  HBRUSH class_brush;
  DWORD style, ex_style;
  int x_pos, y_pos, width, height;
  HWND parent_window;
  HMENU menu;
};

}  // namespace wb::ui::win

#endif  // !WB_WHITEBOX_UI_WIN_WINDOW_DEFINITION_H_
