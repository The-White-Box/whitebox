// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Base UI window.

#ifndef WB_BASE_WIN_UI_BASE_WINDOW_H_
#define WB_BASE_WIN_UI_BASE_WINDOW_H_

#include <sal.h>

#include <cstdint>
#include <memory>
#include <type_traits>

#include "base/base_api.h"
#include "base/base_macroses.h"
#include "base/deps/g3log/g3log.h"
#include "base/win/error_handling/scoped_thread_last_error.h"
#include "base/win/system_error_ext.h"
#include "base/win/ui/scoped_window_class.h"
#include "base/win/ui/window_definition.h"
#include "build/compiler_config.h"

using HINSTANCE = struct HINSTANCE__ *;
using HMENU = struct HMENU__ *;
using HWND = struct HWND__ *;
using WNDPROC = intptr_t(__stdcall *)(HWND, unsigned, uintptr_t, intptr_t);

HWND WB_ATTRIBUTE_DLL_IMPORT __stdcall CreateWindowExA(
    _In_ unsigned long dwExStyle, _In_opt_ const char *lpClassName,
    _In_opt_ const char *lpWindowName, _In_ unsigned long dwStyle, _In_ int X,
    _In_ int Y, _In_ int nWidth, _In_ int nHeight, _In_opt_ HWND hWndParent,
    _In_opt_ HMENU hMenu, _In_opt_ HINSTANCE hInstance, _In_opt_ void *lpParam);

namespace wb::base::windows::ui {

/**
 * @brief Base UI window.
 */
class WB_BASE_API BaseWindow {
 public:
  WB_NO_COPY_CTOR_AND_ASSIGNMENT(BaseWindow);
  BaseWindow(BaseWindow &&w) noexcept;
  BaseWindow &operator=(BaseWindow &&w) noexcept;

  virtual ~BaseWindow() noexcept = 0;

  /**
   * @brief Creates TDerivedWindow window.
   * @tparam TDerivedWindow Window type to create.
   * @param definition Window definition.
   * @param class_style Window class style.
   * @return TDerivedWindow window.
   */
  template <typename TDerivedWindow, typename... Args>
  [[nodiscard]] static std2::result<un<TDerivedWindow>> New(
      _In_ const WindowDefinition &definition, _In_ unsigned long class_style,
      Args &&...args) noexcept {
    static_assert(std::is_base_of_v<BaseWindow, TDerivedWindow>);

    auto window = std::make_unique<TDerivedWindow>(
        definition.instance, definition.icon_id, definition.icon_small_id,
        std::forward<Args>(args)...);
    std::error_code rc{
        RegisterWindowClass<TDerivedWindow>(definition, class_style, window)};
    if (!rc) {
      rc = get_error(::CreateWindowExA(
          definition.ex_style,
          TDerivedWindow::ClassName(definition.name).c_str(), definition.name,
          definition.style, definition.x_pos, definition.y_pos,
          definition.width, definition.height, definition.parent_window,
          definition.menu, definition.instance, window.get()));
    }
    return !rc ? std2::result<un<TDerivedWindow>>{std::move(window)}
               : std2::result<un<TDerivedWindow>>{rc};
  }

  /**
   * @brief Show window with flags.
   * @param flags Flags.
   * @return true if window was visible, false otherwise.
   */
  bool Show(int flags) const noexcept;

  /**
   * @brief Updates the client area of the specified window by sending a
   * WM_PAINT message.
   * @return true if success, false otherwise.
   */
  bool Update() const noexcept;

 protected:
  /**
   * @brief Default ctor.
   * @param instance App instance.
   * @tparam TDerivedWindow.
   */
  BaseWindow(_In_ HINSTANCE instance, _In_ int icon_id,
             _In_ int icon_small_id) noexcept;

  /**
   * @brief Window message handler.  Override in derived windows.
   * @param hwnd Window.
   * @param message Window message.
   * @param wParam Wide message parameter.
   * @param lParam Low message parameter.
   * @return Result code, usually 0 if message was handled.
   */
  virtual intptr_t HandleMessage(_In_ unsigned int message,
                                 _In_ uintptr_t wParam,
                                 _In_ intptr_t lParam) noexcept = 0;

  /**
   * @brief Native window handle.
   * @return Native window handle.
   */
  [[nodiscard]] HWND NativeHandle() const noexcept { return hwnd_; }

 private:
  HINSTANCE instance_;
  HWND hwnd_;

  WB_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()
    // Private member is not accessible to the DLL's client, including inline
    // functions.
    WB_MSVC_DISABLE_WARNING(4251)
    wb::base::un<ScopedWindowClass> scoped_window_class_;
  WB_MSVC_END_WARNING_OVERRIDE_SCOPE()

  /**
   * @brief Static window message handler.  Dispatch window message to
   * actual HandleMessage.
   * @param hwnd Window.
   * @param message Window message.
   * @param wParam Wide message parameter.
   * @param lParam Low message parameter.
   * @return Result code, usually 0 if message was handled.
   */
  template <typename TDerivedWindow>
  static intptr_t CALLBACK WindowMessageHandler(_In_ HWND hwnd,
                                                _In_ unsigned message,
                                                _In_ uintptr_t wParam,
                                                _In_ intptr_t lParam) {
    static_assert(std::is_base_of_v<BaseWindow, TDerivedWindow>);

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
        std2::native_last_errno({});
        const auto rc = ::SetWindowLongPtr(hwnd, GWLP_USERDATA,
                                           reinterpret_cast<LONG_PTR>(window));
        G3CHECK(rc != 0 || !std2::system_last_error_code());
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
  template <typename TDerivedWindow>
  [[nodiscard]] static std::error_code RegisterWindowClass(
      _In_ const WindowDefinition &definition,
      _In_ const unsigned long class_style,
      _In_ const wb::base::un<TDerivedWindow> &window) noexcept {
    static_assert(std::is_base_of_v<BaseWindow, TDerivedWindow>);

    auto new_scoped_window_class =
        CreateWindowClass(definition, class_style,
                          TDerivedWindow::ClassName(definition.name).c_str(),
                          &WindowMessageHandler<TDerivedWindow>);

    G3DCHECK(!!window);

    auto &scoped_window_class = window->scoped_window_class_;
    scoped_window_class.swap(new_scoped_window_class);

    return scoped_window_class->error_code();
  }

  /**
   * @brief Creates window class.
   * @tparam TDerivedWindow Window type.
   * @param definition Window definition.
   * @param class_style Window class style.
   * @param class_name Window class name.
   * @param window_message_handler Window message handler.
   * @return ScopedWindowClass.
   */
  [[nodiscard]] static wb::base::un<ScopedWindowClass> CreateWindowClass(
      _In_ const WindowDefinition &definition,
      _In_ const unsigned long class_style, _In_ const char *class_name,
      _In_ WNDPROC window_message_handler) noexcept;

 protected:
  int icon_id_, icon_small_id_;
};

}  // namespace wb::base::windows::ui

#endif  // !WB_BASE_WIN_UI_BASE_WINDOW_H_
