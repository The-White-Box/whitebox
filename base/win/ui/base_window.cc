// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Base UI window.

#include "base_window.h"

#include "base/base_macroses.h"
#include "base/deps/g3log/g3log.h"
#include "base/win/error_handling/scoped_thread_last_error.h"
#include "base/win/system_error_ext.h"
#include "base/win/ui/scoped_window_class.h"
#include "base/win/ui/window_utilities.h"
#include "base/win/windows_light.h"

namespace wb::base::windows::ui {
BaseWindow::BaseWindow(BaseWindow &&w) noexcept
    : instance_{w.instance_},
      hwnd_{w.hwnd_},
      scoped_window_class_{std::move(w.scoped_window_class_)},
      icon_id_{w.icon_id_},
      icon_small_id_{w.icon_small_id_} {
  w.instance_ = nullptr;
  w.hwnd_ = nullptr;
}

BaseWindow &BaseWindow::operator=(BaseWindow &&w) noexcept {
  std::swap(instance_, w.instance_);
  std::swap(hwnd_, w.hwnd_);
  std::swap(scoped_window_class_, w.scoped_window_class_);
  std::swap(icon_id_, w.icon_id_);
  std::swap(icon_small_id_, w.icon_small_id_);
  return *this;
}

BaseWindow::~BaseWindow() noexcept = default;

BaseWindow::BaseWindow(_In_ HINSTANCE instance, _In_ int icon_id,
                       _In_ int icon_small_id) noexcept
    : instance_{instance},
      hwnd_{nullptr},
      icon_id_{icon_id},
      icon_small_id_{icon_small_id} {}

bool BaseWindow::Show(int flags) const noexcept {
  G3DCHECK(!!hwnd_);
  return !!::ShowWindow(hwnd_, flags);
}

bool BaseWindow::Update() const noexcept {
  G3DCHECK(!!hwnd_);
  return !!::UpdateWindow(hwnd_);
}

[[nodiscard]] wb::base::un<ScopedWindowClass> BaseWindow::CreateWindowClass(
    _In_ const WindowDefinition &definition,
    _In_ const unsigned long class_style, _In_ const char *class_name,
    _In_ WNDPROC window_message_handler) noexcept {
  G3DCHECK(!!class_name);
  G3DCHECK(!!window_message_handler);

  const auto icon =
      LoadIcon(definition.instance,
               wb::base::windows::ui::MakeIntResource(definition.icon_id));
  G3DCHECK(!!icon);

  const auto icon_small = LoadIcon(
      definition.instance,
      wb::base::windows::ui::MakeIntResource(definition.icon_small_id));
  G3DCHECK(!!icon_small);

  WNDCLASSEX wnd_class = {sizeof(wnd_class)};
  wnd_class.style = class_style;
  wnd_class.lpfnWndProc = window_message_handler;
  wnd_class.cbClsExtra = 0;
  wnd_class.cbWndExtra = 0;
  wnd_class.hInstance = definition.instance;
  wnd_class.hIcon = icon;
  wnd_class.hCursor = definition.cursor;
  wnd_class.hbrBackground = definition.class_brush;
  wnd_class.lpszClassName = class_name;
  wnd_class.hIconSm = icon_small;

  return std::make_unique<ScopedWindowClass>(wnd_class.hInstance, wnd_class);
}
}  // namespace wb::base::windows::ui
