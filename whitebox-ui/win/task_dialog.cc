// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Windows Task Dialog.

#include "task_dialog.h"

#include <cstdint>

#include "base/base_macroses.h"
#include "base/deps/g3log/g3log.h"
#include "base/std2/string_ext.h"
#include "base/win/system_error_ext.h"
#include "base/win/windows_light.h"
#include "whitebox-ui/win/window_message_handlers.h"
#include "whitebox-ui/win/window_utilities.h"
//
#include <Commctrl.h>
#include <shellapi.h>

namespace {

/**
 * @brief Get task dialog icon by dialog kind.
 * @param kind Dialog kind.
 * @return Task dialog icon resource.
 */
constexpr wchar_t* GetIconByKind(wb::ui::win::DialogBoxKind kind) noexcept {
  switch (kind) {
    case wb::ui::win::DialogBoxKind::kInformation:
      // NOLINTNEXTLINE(performance-no-int-to-ptr): System header define.
      return TD_INFORMATION_ICON;
    case wb::ui::win::DialogBoxKind::kWarning:
      // NOLINTNEXTLINE(performance-no-int-to-ptr): System header define.
      return TD_WARNING_ICON;
    case wb::ui::win::DialogBoxKind::kError:
      // NOLINTNEXTLINE(performance-no-int-to-ptr): System header define.
      return TD_ERROR_ICON;
    case wb::ui::win::DialogBoxKind::kShield:
      // NOLINTNEXTLINE(performance-no-int-to-ptr): System header define.
      return TD_SHIELD_ICON;
    default:
      G3DLOG(FATAL) << "Unknown dialog box kind: "
                    << wb::base::underlying_cast(kind);
      // NOLINTNEXTLINE(performance-no-int-to-ptr): System header define.
      return TD_INFORMATION_ICON;
  }
}

/**
 * @brief Get dialog box button by its id.
 * @param button_id Button id.
 * @return Dialog box button
 */
constexpr wb::ui::win::DialogBoxButton GetButtonById(int button_id) noexcept {
  switch (button_id) {
    case IDOK:
      return wb::ui::win::DialogBoxButton::kOk;
    case IDCANCEL:
      return wb::ui::win::DialogBoxButton::kCancel;
    case IDRETRY:
      return wb::ui::win::DialogBoxButton::kRetry;
    case IDYES:
      return wb::ui::win::DialogBoxButton::kYes;
    case IDNO:
      return wb::ui::win::DialogBoxButton::kNo;
    case IDCLOSE:
      return wb::ui::win::DialogBoxButton::kClose;
    default:
      G3DLOG(FATAL) << "Unknown dialog box button id: " << button_id;
      return wb::ui::win::DialogBoxButton::kCancel;
  }
}

/**
 * @brief Set task dialog icon.
 * @param window Task dialog window.
 * @param icon_id Icon id.
 * @return void.
 */
template <int icon_type>
inline void SetTaskDialogIcon(_In_ HWND window, _In_ int icon_id) noexcept {
  const HANDLE exe_icon{::LoadImage(::GetModuleHandle(nullptr),
                                    wb::ui::win::MakeIntResource(icon_id),
                                    IMAGE_ICON, 0, 0, LR_DEFAULTSIZE)};
  G3DCHECK(!!exe_icon);

  ::SendMessage(window, WM_SETICON, icon_type,
                reinterpret_cast<LPARAM>(exe_icon));
}

/**
 * @brief Task dialog context.
 */
struct TaskDialogContext {
  /**
   * @brief Main icon id.
   */
  int main_icon_id;
  /**
   * @brief Small icon id.
   */
  int small_icon_id;
};

/**
 * @brief Handles task dialog constructed message.
 * @param window Task dialog window.
 * @param Context.
 * @return void.
 */
inline void OnTaskDialogConstructed(_In_ HWND window,
                                    _In_ LONG_PTR ctx) noexcept {
  G3DCHECK(!!window);

  // NOLINTNEXTLINE(performance-no-int-to-ptr): API design.
  const auto* context = reinterpret_cast<TaskDialogContext*>(ctx);
  G3DCHECK(!!context);

  // Need nice app icons for task dialog.
  SetTaskDialogIcon<ICON_BIG>(window, context->main_icon_id);
  SetTaskDialogIcon<ICON_SMALL>(window, context->small_icon_id);
}

/**
 * @brief Handles task dialog hyperlink clicked message.
 * @param window Task dialog.
 * @param Context.
 * @param url Hyperlink url.
 * @return void.
 */
inline void OnTaskDialogHyperlinkClicked(_In_ HWND window, _In_ LONG_PTR,
                                         _In_z_ const wchar_t* url) noexcept {
  G3DCHECK(!!window);
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
    // NOLINTNEXTLINE(performance-no-int-to-ptr): API design.
    HANDLE_TD_MSG(hwnd, TDN_DIALOG_CONSTRUCTED, OnTaskDialogConstructed);
    // NOLINTNEXTLINE(performance-no-int-to-ptr): API design.
    HANDLE_TD_MSG(hwnd, TDN_HYPERLINK_CLICKED, OnTaskDialogHyperlinkClicked);
  }
  return S_OK;
}

}  // namespace

namespace wb::ui::win {

/**
 * @brief Shows dialog box.  The parent window should not be hidden or disabled
 * when this function is called.  Requires COM to be initialized before.
 * @param kind Dialog box kind.
 * @param settings Dialog box settings.
 * @return true on success, false on failure.
 */
WB_WHITEBOX_UI_API base::std2::result<DialogBoxButton> ShowDialogBox(
    DialogBoxKind kind, const DialogBoxSettings& settings) noexcept {
  using namespace wb::base;
  using namespace wb::ui;

  const std::wstring title{std2::UTF8ToWide(settings.title)};
  const std::wstring main_instruction{
      std2::UTF8ToWide(settings.main_instruction)};
  const std::wstring content{std2::UTF8ToWide(settings.content)};

  const auto no_collapse_settings =
      DialogBoxCollapseSettings{std::string{}, std::string{}, std::string{}};
  const auto& collapse_settings = settings.collapse_settings.has_value()
                                      ? settings.collapse_settings.value()
                                      : no_collapse_settings;
  const std::wstring expanded_control_text{
      std2::UTF8ToWide(collapse_settings.expanded_control_text)};
  const std::wstring collapsed_control_text{
      std2::UTF8ToWide(collapse_settings.collapsed_control_text)};
  const std::wstring expanded_content{
      std2::UTF8ToWide(collapse_settings.expand_collapse_content)};

  const std::wstring footer{std2::UTF8ToWide(settings.footer_text)};
  const TaskDialogContext context{.main_icon_id = settings.main_icon_id,
                                  .small_icon_id = settings.small_icon_id};

  const TASKDIALOGCONFIG config{
      .cbSize = static_cast<unsigned>(sizeof(config)),
      .hwndParent = settings.parent_window,
      .hInstance = nullptr,
      .dwFlags = TDF_ENABLE_HYPERLINKS | TDF_ALLOW_DIALOG_CANCELLATION |
                 TDF_POSITION_RELATIVE_TO_WINDOW | TDF_SIZE_TO_CONTENT |
                 (settings.rtl_layout ? TDF_RTL_LAYOUT : 0),
      .dwCommonButtons = implicit_cast<TASKDIALOG_COMMON_BUTTON_FLAGS>(
          underlying_cast(settings.buttons)),
      .pszWindowTitle = title.c_str(),
      .pszMainIcon = GetIconByKind(kind),
      .pszMainInstruction = main_instruction.c_str(),
      .pszContent = content.c_str(),
      .nDefaultButton = (settings.buttons & DialogBoxButton::kCancel) ==
                                DialogBoxButton::kCancel
                            ? IDCANCEL
                            : IDOK,
      .pszExpandedInformation = std2::CStringOrNullptrIfEmpty(expanded_content),
      .pszExpandedControlText =
          std2::CStringOrNullptrIfEmpty(expanded_control_text),
      .pszCollapsedControlText =
          std2::CStringOrNullptrIfEmpty(collapsed_control_text),
      .pszFooterIcon = GetIconByKind(DialogBoxKind::kInformation),
      .pszFooter = footer.c_str(),
      .pfCallback = &ShowDialogBoxCallback,
      .lpCallbackData = reinterpret_cast<LONG_PTR>(&context),
      .cxWidth = 0};

  int pressed_button_id;
  const std::error_code rc{wb::base::win::get_error(
      ::TaskDialogIndirect(&config, &pressed_button_id, nullptr, nullptr))};

  G3DPCHECK_E(!rc, rc) << "TaskDialog can't be shown.";
  return !rc ? std2::result<DialogBoxButton>{GetButtonById(pressed_button_id)}
             : std2::result<DialogBoxButton>{rc};
}

}  // namespace wb::ui::win
