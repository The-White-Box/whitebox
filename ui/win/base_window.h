// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Base UI window.

#ifndef WB_UI_WIN_BASE_WINDOW_H_
#define WB_UI_WIN_BASE_WINDOW_H_

#include <sal.h>

#include <cstdint>
#include <memory>
#include <type_traits>

#include "base/deps/g3log/g3log.h"
#include "base/macroses.h"
#include "base/win/error_handling/scoped_thread_last_error.h"
#include "base/win/system_error_ext.h"
#include "build/compiler_config.h"
#include "ui/config.h"
#include "ui/win/scoped_window_class.h"
#include "ui/win/window_definition.h"

using HINSTANCE = struct HINSTANCE__ *;
using HMENU = struct HMENU__ *;
using HWND = struct HWND__ *;
using WNDPROC = intptr_t(__stdcall *)(HWND, unsigned, uintptr_t, intptr_t);

HWND WB_ATTRIBUTE_DLL_IMPORT __stdcall CreateWindowExA(
    _In_ unsigned long dwExStyle, _In_opt_ const char *lpClassName,
    _In_opt_ const char *lpWindowName, _In_ unsigned long dwStyle, _In_ int X,
    _In_ int Y, _In_ int nWidth, _In_ int nHeight, _In_opt_ HWND hWndParent,
    _In_opt_ HMENU hMenu, _In_opt_ HINSTANCE hInstance, _In_opt_ void *lpParam);

namespace wb::ui::win {

/**
 * @brief Base UI window.
 */
class WB_WHITEBOX_UI_API BaseWindow {
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
  [[nodiscard]] static base::std2::result<base::un<TDerivedWindow>> New(
      _In_ const WindowDefinition &definition, _In_ unsigned long class_style,
      Args &&...args) noexcept {
    static_assert(std::is_base_of_v<BaseWindow, TDerivedWindow>);

    using namespace base;
    using namespace base::win;

    auto window = std::make_unique<TDerivedWindow>(
        definition.instance, definition.icon_id, definition.icon_small_id,
        std::forward<Args>(args)...);
    std::error_code rc{
        RegisterWindowClass<TDerivedWindow>(definition, class_style, window)};
    if (!rc) {
      const std::string class_name{TDerivedWindow::ClassName(definition.name)};
      rc = get_error(::CreateWindowExA(
          definition.ex_style, class_name.c_str(), definition.name,
          definition.style, definition.x_pos, definition.y_pos,
          definition.width, definition.height, definition.parent_window,
          definition.menu, definition.instance, window.get()));
    }
    return !rc ? std2::result<un<TDerivedWindow>>{std::move(window)}
               : std2::result<un<TDerivedWindow>>{std::unexpect, rc};
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
    base::un<ScopedWindowClass> scoped_window_class_;
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

    using namespace wb::base;

    TDerivedWindow *self{nullptr};

    switch (message) {
      // Set up the self before handling WM_NCCREATE.
      case WM_NCCREATE: {
        auto *create_struct = reinterpret_cast<CREATESTRUCT *>(lParam);
        G3CHECK(!!create_struct);

        self = static_cast<TDerivedWindow *>(create_struct->lpCreateParams);
        G3CHECK(!!self);

        SetWindowUserData(hwnd, self);

        G3DCHECK(!!hwnd);
        self->hwnd_ = hwnd;
      } break;

      // Clear the pointer to stop calling the self once WM_NCDESTROY is
      // received.
      case WM_NCDESTROY:
        SetWindowUserData(hwnd, nullptr);
        break;

      default: {
        self = reinterpret_cast<TDerivedWindow *>(
            ::GetWindowLongPtr(hwnd, GWLP_USERDATA));
      } break;
    }

    return self ? self->HandleMessage(message, wParam, lParam)
                : ::DefWindowProc(hwnd, message, wParam, lParam);
  }

  /**
   * @brief Set window user data.
   * @param hwnd Window.
   * @param user_data Data to set.
   * @return void.
   */
  static void SetWindowUserData(_In_ HWND hwnd,
                                _In_opt_ void *user_data) noexcept {
    G3DCHECK(!!hwnd);

    base::win::error_handling::ScopedThreadLastError last_error_restorer;

    // To determine success or failure, clear the last error information
    // by calling SetLastError with 0, then call SetWindowLongPtr.
    base::std2::native_last_errno({});

    const LONG_PTR rc{::SetWindowLongPtr(
        hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(user_data))};

    // Function failure will be indicated by a return value of zero and a
    // GetLastError result that is nonzero.
    G3CHECK(rc != 0 || !base::std2::system_last_error_code());
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
      _In_ const base::un<TDerivedWindow> &window) noexcept {
    static_assert(std::is_base_of_v<BaseWindow, TDerivedWindow>);

    const auto class_name = TDerivedWindow::ClassName(definition.name);
    auto new_scoped_window_class =
        CreateWindowClass(definition, class_style, class_name.c_str(),
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
  [[nodiscard]] static base::un<ScopedWindowClass> CreateWindowClass(
      _In_ const WindowDefinition &definition,
      _In_ const unsigned long class_style, _In_ const char *class_name,
      _In_ WNDPROC window_message_handler) noexcept;

 protected:
  int icon_id_, icon_small_id_;
};

}  // namespace wb::ui::win

#endif  // !WB_UI_WIN_BASE_WINDOW_H_
