// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Base UI window.

#ifndef WB_BASE_WINDOWS_UI_BASE_WINDOW_H_
#define WB_BASE_WINDOWS_UI_BASE_WINDOW_H_

#include <tchar.h>

#include <type_traits>

#include "base/base_macroses.h"
#include "base/deps/g3log/g3log.h"
#include "base/windows/error_handling/scoped_thread_last_error.h"
#include "base/windows/system_error_ext.h"
#include "base/windows/ui/scoped_window_class.h"
#include "base/windows/windows_light.h"

namespace wb::base::windows::ui {
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
  WindowDefinition(_In_opt_ HINSTANCE instance_, _In_opt_ LPCTSTR name_,
                   _In_ int icon_id_, _In_ int icon_small_id_,
                   _In_opt_ HCURSOR cursor_, _In_opt_ HBRUSH class_brush_,
                   _In_ DWORD style_, _In_ DWORD ex_style_ = 0,
                   _In_ int x_pos_ = CW_USEDEFAULT,
                   _In_ int y_pos_ = CW_USEDEFAULT,
                   _In_ int width_ = CW_USEDEFAULT,
                   _In_ int height_ = CW_USEDEFAULT,
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
  LPCTSTR name;
  int icon_id, icon_small_id;
  HCURSOR cursor;
  HBRUSH class_brush;
  DWORD style, ex_style;
  int x_pos, y_pos, width, height;
  HWND parent_window;
  HMENU menu;
};

/**
 * @brief Base UI window.
 * @param definition Window definition
 * @param class_style
 * @return
 */
template <typename TDerivedWindow>
class BaseWindow {
  // static_assert(std::is_base_of_v<BaseWindow<TDerivedWindow>,
  // TDerivedWindow>);

 public:
  WB_NO_COPY_CTOR_AND_ASSIGNMENT(BaseWindow);

  /**
   * @brief Move ctor.
   * @tparam TDerivedWindow
   */
  BaseWindow(BaseWindow &&w) noexcept
      : instance_{w.instance_},
        hwnd_{w.hwnd_},
        scoped_window_class_{std::move(w.scoped_window_class_)},
        icon_id_{w.icon_id_},
        icon_small_id_{w.icon_small_id_} {
    w.instance_ = nullptr;
    w.hwnd_ = nullptr;
  }

  /**
   * @brief Move assingment.
   * @tparam TDerivedWindow
   */
  BaseWindow &operator=(BaseWindow &&w) noexcept {
    std::swap(instance_, w.instance_);
    std::swap(hwnd_, w.hwnd_);
    std::swap(scoped_window_class_, w.scoped_window_class_);
    std::swap(icon_id_, w.icon_id_);
    std::swap(icon_small_id_, w.icon_small_id_);
    return *this;
  }

  /**
   * @brief Dtor.
   * @tparam TDerivedWindow
   */
  virtual ~BaseWindow() noexcept = 0;

  /**
   * @brief Creates window.
   * @tparam TDerivedWindow
   */
  [[nodiscard]] static std_ext::os_res<un<TDerivedWindow>> Create(
      _In_ const WindowDefinition &definition,
      _In_ DWORD class_style) noexcept {
    auto window = std::make_unique<TDerivedWindow>(
        definition.instance, definition.icon_id, definition.icon_small_id);
    std::error_code rc{RegisterWindowClass(
        definition, class_style, TDerivedWindow::ClassName(), window)};
    if (!rc) {
      rc = GetErrorCode(::CreateWindowEx(
          definition.ex_style, TDerivedWindow::ClassName(), definition.name,
          definition.style, definition.x_pos, definition.y_pos,
          definition.width, definition.height, definition.parent_window,
          definition.menu, definition.instance, window.get()));
    }
    return !rc ? std_ext::os_res<un<TDerivedWindow>>{std::move(window)}
               : std_ext::os_res<un<TDerivedWindow>>{rc};
  }

  int Show(int flags) const noexcept {
    G3DCHECK(!!hwnd_);
    return ::ShowWindow(hwnd_, flags);
  }

  int Update() const noexcept {
    G3DCHECK(!!hwnd_);
    return ::UpdateWindow(hwnd_);
  }

 protected:
  /**
   * @brief Default ctor.
   * @param instance App instance.
   * @tparam TDerivedWindow.
   */
  BaseWindow(_In_ HINSTANCE instance, _In_ int icon_id,
             _In_ int icon_small_id) noexcept
      : instance_{instance},
        hwnd_{nullptr},
        icon_id_{icon_id},
        icon_small_id_{icon_small_id} {}

  /**
   * @brief Window message handler.  Override in derived windows.
   * @param hwnd Window.
   * @param message Window message.
   * @param wParam Wide message parameter.
   * @param lParam Low message parameter.
   * @return Result code, usually 0 if message was handled.
   */
  virtual LRESULT HandleMessage(_In_ UINT message, _In_ WPARAM wParam,
                                _In_ LPARAM lParam) noexcept = 0;

  /**
   * @brief Native window handle.
   * @return Native window handle.
   */
  [[nodiscard]] HWND NativeHandle() const noexcept { return hwnd_; }

 private:
  HINSTANCE instance_;
  HWND hwnd_;
  std::unique_ptr<ScopedWindowClass> scoped_window_class_;

  /**
   * @brief Static window message handler.  Dispatch window message to
   * actual HandleMessage.
   * @param hwnd Window.
   * @param message Window message.
   * @param wParam Wide message parameter.
   * @param lParam Low message parameter.
   * @return Result code, usually 0 if message was handled.
   */
  static LRESULT CALLBACK WindowMessageHandler(_In_ HWND hwnd,
                                               _In_ UINT message,
                                               _In_ WPARAM wParam,
                                               _In_ LPARAM lParam) {
    TDerivedWindow *window{nullptr};

    if (message == WM_NCCREATE) {
      auto *create_struct = reinterpret_cast<CREATESTRUCT *>(lParam);
      G3CHECK(!!create_struct);

      window = static_cast<TDerivedWindow *>(create_struct->lpCreateParams);
      G3CHECK(!!window);

      {
        error_handling::ScopedThreadLastError restore_last_error_on_out;

        // To determine success or failure, clear the last error information by
        // calling SetLastError with 0, then call SetWindowLongPtr.  Function
        // failure will be indicated by a return value of zero and a
        // GetLastError result that is nonzero.
        std_ext::SetThreadErrorCode({});
        const auto rc = ::SetWindowLongPtr(hwnd, GWLP_USERDATA,
                                           reinterpret_cast<LONG_PTR>(window));
        G3CHECK(rc != 0 || !std_ext::GetThreadErrorCode());
      }

      G3DCHECK(!!hwnd);
      window->hwnd_ = hwnd;
    } else {
      window = reinterpret_cast<TDerivedWindow *>(
          ::GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    return window ? window->HandleMessage(message, wParam, lParam)
                  : ::DefWindowProc(hwnd, message, wParam, lParam);
  }

  /**
   * @brief Registers window class.
   * @param definition Window definition.
   * @param class_style Window class style.
   * @param class_name Window class name.
   * @param window Window to put class to.
   * @return Registration result code.
   */
  [[nodiscard]] static std::error_code RegisterWindowClass(
      _In_ const WindowDefinition &definition, _In_ const DWORD class_style,
      _In_ LPCTSTR class_name,
      _In_ const std::unique_ptr<TDerivedWindow> &window) noexcept {
    G3DCHECK(!!class_name);

    const auto icon =
        LoadIcon(definition.instance, MAKEINTRESOURCE(definition.icon_id));
    G3DCHECK(!!icon);

    const auto icon_small = LoadIcon(definition.instance,
                                     MAKEINTRESOURCE(definition.icon_small_id));
    G3DCHECK(!!icon_small);

    WNDCLASSEX wnd_class = {sizeof(wnd_class)};
    wnd_class.style = class_style;
    wnd_class.lpfnWndProc = TDerivedWindow::WindowMessageHandler;
    wnd_class.cbClsExtra = 0;
    wnd_class.cbWndExtra = 0;
    wnd_class.hInstance = definition.instance;
    wnd_class.hIcon = icon;
    wnd_class.hCursor = definition.cursor;
    wnd_class.hbrBackground = definition.class_brush;
    wnd_class.lpszClassName = class_name;
    wnd_class.hIconSm = icon_small;

    G3DCHECK(!!window);
    auto &scoped_window_class = window->scoped_window_class_;
    scoped_window_class.reset(new ScopedWindowClass{wnd_class});

    return scoped_window_class->error_code();
  }

 protected:
  int icon_id_, icon_small_id_;
};

template <typename TDerivedWindow>
BaseWindow<TDerivedWindow>::~BaseWindow() noexcept {}
}  // namespace wb::base::windows::ui

#endif  // !WB_BASE_WINDOWS_UI_BASE_WINDOW_H_
