// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Windows Task Dialog.

#include "task_dialog.h"

#include <cstdint>

#include "base/base_macroses.h"
#include "base/deps/g3log/g3log.h"
#include "base/std_ext/cstring_ext.h"
#include "base/windows/system_error_ext.h"
#include "base/windows/ui/window_message_handlers.h"
#include "base/windows/windows_light.h"
//
#include <Commctrl.h>
#include <shellapi.h>

namespace {
/**
 * @brief Get task dialog icon by dialog kind.
 * @param kind Dialog kind.
 * @return Task dialog icon resource.
 */
constexpr wchar_t* GetIconByKind(
    wb::base::windows::ui::DialogBoxKind kind) noexcept {
  switch (kind) {
    case wb::base::windows::ui::DialogBoxKind::kInformation:
      return TD_INFORMATION_ICON;
    case wb::base::windows::ui::DialogBoxKind::kWarning:
      return TD_WARNING_ICON;
    case wb::base::windows::ui::DialogBoxKind::kError:
      return TD_ERROR_ICON;
    case wb::base::windows::ui::DialogBoxKind::kShield:
      return TD_SHIELD_ICON;
    default:
      G3LOG(FATAL) << "Unknown dialog box kind: "
                   << wb::base::underlying_cast(kind);
      return TD_INFORMATION_ICON;
  }
}

/**
 * @brief Get dialog box button by its id.
 * @param button_id Button id.
 * @return Dialog box button
 */
constexpr wb::base::windows::ui::DialogBoxButton GetButtonById(
    int button_id) noexcept {
  switch (button_id) {
    case IDOK:
      return wb::base::windows::ui::DialogBoxButton::kOk;
    case IDCANCEL:
      return wb::base::windows::ui::DialogBoxButton::kCancel;
    case IDRETRY:
      return wb::base::windows::ui::DialogBoxButton::kRetry;
    case IDYES:
      return wb::base::windows::ui::DialogBoxButton::kYes;
    case IDNO:
      return wb::base::windows::ui::DialogBoxButton::kNo;
    case IDCLOSE:
      return wb::base::windows::ui::DialogBoxButton::kClose;
    default:
      G3LOG(FATAL) << "Unknown dialog box button id: " << button_id;
      return wb::base::windows::ui::DialogBoxButton::kCancel;
  }
}

/**
 * @brief Set task dialog icon.
 * @param window Task dialog window.
 * @return void.
 */
template <int icon_type, int icon_id>
void SetTaskDialogIcon(HWND window) noexcept {
  const HANDLE exe_icon{::LoadImage(::GetModuleHandle(nullptr),
                                    MAKEINTRESOURCE(icon_id), IMAGE_ICON, 0, 0,
                                    LR_DEFAULTSIZE)};
  G3DCHECK(!!exe_icon);

  ::SendMessage(window, WM_SETICON, icon_type,
                reinterpret_cast<LPARAM>(exe_icon));
}

/**
 * @brief Handles task dialog constructed message.
 * @param window Task dialog window.
 * @param Context.
 * @return void.
 */
void OnTaskDialogConstructed(HWND window, LONG_PTR) noexcept {
  G3DCHECK(!!window);

  // Need nice app icons for task dialog.
  SetTaskDialogIcon<ICON_BIG, 101>(window);
  SetTaskDialogIcon<ICON_SMALL, 102>(window);
}

/**
 * @brief Handles task dialog hyperlink clicked message.
 * @param window Task dialog.
 * @param Context.
 * @param url Hyperlink url.
 * @return void.
 */
void OnTaskDialogHyperlinkClicked(HWND window, LONG_PTR,
                                  const wchar_t* url) noexcept {
  G3DCHECK(!!url);

  // "If the function succeeds, it returns a value greater than 32.  If the
  // function fails, it returns an error value that indicates the cause of the
  // failure.  The return value is cast as an HINSTANCE for backward
  // compatibility with 16-bit Windows applications. It is not a true HINSTANCE,
  // however.  It can be cast only to an int and compared to either 32 or the
  // following error codes below."
  //
  // See
  // https://docs.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-shellexecutew
  const int rc{static_cast<int>(reinterpret_cast<std::intptr_t>(
      ::ShellExecuteW(window, L"open", url, nullptr, nullptr, SW_SHOWNORMAL)))};
  // Well, if browser can't run, we can't do anything meaningful.
  G3LOGF_IF(WARNING, rc <= 32, "ShellExecuteW failed to open url %S w/e: %d",
            url, rc);
}

/**
 * @brief Callback for task dialog main procedure.
 * @param hwnd Task dialog window.
 * @param msg Message.
 * @param wParam Wide param.
 * @param lParam Low param.
 * @param lpRefData Context.
 * @return Callback result code.  Depends on msg.
 */
HRESULT __stdcall ShowDialogBoxCallback(_In_ HWND hwnd, _In_ UINT msg,
                                        _In_ [[maybe_unused]] WPARAM wParam,
                                        _In_ [[maybe_unused]] LPARAM lParam,
                                        _In_ LONG_PTR lpRefData) {
  switch (msg) {
    HANDLE_TD_MSG(hwnd, TDN_DIALOG_CONSTRUCTED, OnTaskDialogConstructed);
    HANDLE_TD_MSG(hwnd, TDN_HYPERLINK_CLICKED, OnTaskDialogHyperlinkClicked);
  }
  return S_OK;
}
}  // namespace

namespace wb::base::windows::ui {
/**
 * @brief Shows dialog box.  The parent window should not be hidden or disabled
 * when this function is called.  Requires COM to be initialized before.
 * @param kind Dialog box kind.
 * @param settings Dialog box settings.
 * @return true on success, false on failure.
 */
WB_BASE_API std_ext::sc_res<DialogBoxButton> ShowDialogBox(
    DialogBoxKind kind, const DialogBoxSettings& settings) noexcept {
  const std::wstring title{std_ext::AnsiToWide(settings.title)};
  const std::wstring main_instruction{
      std_ext::AnsiToWide(settings.main_instruction)};
  const std::wstring content{std_ext::AnsiToWide(settings.content)};
  const std::wstring expanded_control_text{
      std_ext::AnsiToWide(settings.expanded_control_text)};
  const std::wstring collapsed_control_text{
      std_ext::AnsiToWide(settings.collapsed_control_text)};
  const std::wstring expanded_content{
      std_ext::AnsiToWide(settings.expand_collapse_content)};
  const std::wstring footer{std_ext::AnsiToWide(settings.footer_text)};

  const TASKDIALOGCONFIG config{
      .cbSize = static_cast<unsigned>(sizeof(config)),
      .hwndParent = settings.parent_window,
      .hInstance = nullptr,
      .dwFlags = TDF_ENABLE_HYPERLINKS | TDF_ALLOW_DIALOG_CANCELLATION |
                 TDF_POSITION_RELATIVE_TO_WINDOW | TDF_SIZE_TO_CONTENT |
                 (settings.rtl_layout ? TDF_RTL_LAYOUT : 0),
      .dwCommonButtons =
          implicit_cast<TASKDIALOG_COMMON_BUTTON_FLAGS>(settings.buttons),
      .pszWindowTitle = title.c_str(),
      .pszMainIcon = GetIconByKind(kind),
      .pszMainInstruction = main_instruction.c_str(),
      .pszContent = content.c_str(),
      .nDefaultButton = (settings.buttons & DialogBoxButton::kCancel) ==
                                DialogBoxButton::kCancel
                            ? IDCANCEL
                            : IDOK,
      .pszExpandedInformation = expanded_content.c_str(),
      .pszExpandedControlText = expanded_control_text.c_str(),
      .pszCollapsedControlText = collapsed_control_text.c_str(),
      .pszFooterIcon = GetIconByKind(DialogBoxKind::kInformation),
      .pszFooter = footer.c_str(),
      .pfCallback = &ShowDialogBoxCallback,
      .lpCallbackData = 0,
      .cxWidth = 0};

  int pressed_button_id;
  const std::error_code rc{GetErrorCode(
      ::TaskDialogIndirect(&config, &pressed_button_id, nullptr, nullptr))};

  G3DCHECK(!rc) << "TaskDialog can't be shown: " << rc.message();
  return !rc ? std_ext::sc_res<DialogBoxButton>{GetButtonById(
                   pressed_button_id)}
             : std_ext::sc_res<DialogBoxButton>{rc};
}
}  // namespace wb::base::windows::ui
