// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Windows Task Dialog.

#ifndef WB_BASE_WINDOWS_UI_TASK_DIALOG_H_
#define WB_BASE_WINDOWS_UI_TASK_DIALOG_H_

#include <sal.h>

#include <cstddef>  // std::byte
#include <string>
#include <variant>

#include "base/base_api.h"
#include "base/base_macroses.h"
#include "base/std_ext/system_error_ext.h"

using HWND = struct HWND__ *;

namespace wb::base::windows::ui {
/**
 * @brief Kind of dialog box.
 */
enum class DialogBoxKind { kInformation, kWarning, kError, kShield };

/**
 * @brief Dialog box buttons.  Should be same as TASKDIALOG_COMMON_BUTTON_FLAGS.
 */
enum class DialogBoxButton {
  kOk = 0x01,
  kYes = 0x02,
  kNo = 0x04,
  kCancel = 0x08,
  kRetry = 0x10,
  kClose = 0x20
};

/**
 * @brief operator| for DialogBoxButton.
 * @param left Left.
 * @param right Right.
 * @return Left | Right.
 */
[[nodiscard]] constexpr DialogBoxButton operator|(
    DialogBoxButton left, DialogBoxButton right) noexcept {
  return static_cast<DialogBoxButton>(underlying_cast(left) |
                                      underlying_cast(right));
}

/**
 * @brief operator& for DialogBoxButton.
 * @param left Left.
 * @param right Right.
 * @return Left & Right.
 */
[[nodiscard]] constexpr DialogBoxButton operator&(
    DialogBoxButton left, DialogBoxButton right) noexcept {
  return static_cast<DialogBoxButton>(underlying_cast(left) &
                                      underlying_cast(right));
}

/**
 * @brief Dialog box settings.
 */
struct DialogBoxSettings {
  /**
   * @brief Creates dialog box settings.
   * @param parent_window_ Parent window.  May be nullptr.
   * @param title_ Dialog box title.
   * @param main_instruction_ Main instruction.
   * @param content_ Content.
   * @param expanded_control_text_ Expanded toggle control text.
   * @param collapsed_control_text_ Collapsed toggle control text.
   * @param expand_collapse_content_ Expand control content when toggle
   * expanded.
   * @param footer_text_ Footer text.
   * @param rtl_layout_ Is Left to right text layout or not?
   * @param is_cancellable_ Is cancel button available?
   * @return nothing.
   */
  DialogBoxSettings(_In_opt_ HWND parent_window_, std::string title_,
                    std::string main_instruction_, std::string content_,
                    std::string expanded_control_text_,
                    std::string collapsed_control_text_,
                    std::string expand_collapse_content_,
                    std::string footer_text_, DialogBoxButton buttons_,
                    bool rtl_layout_) noexcept
      : parent_window{parent_window_},
        title{std::move(title_)},
        main_instruction{std::move(main_instruction_)},
        content{std::move(content_)},
        expanded_control_text{std::move(expanded_control_text_)},
        collapsed_control_text{std::move(collapsed_control_text_)},
        expand_collapse_content{std::move(expand_collapse_content_)},
        footer_text{std::move(footer_text_)},
        buttons{buttons_},
        rtl_layout{rtl_layout_} {}

  /**
   * @brief Parent window.  Optional.
   */
  const HWND parent_window;
  /**
   * @brief Title.
   */
  const std::string title;
  /**
   * @brief Main instruction.
   */
  const std::string main_instruction;
  /**
   * @brief Content.
   */
  const std::string content;
  /**
   * @brief Expanded toggle control text.
   */
  const std::string expanded_control_text;
  /**
   * @brief Collapsed toggle control text.
   */
  const std::string collapsed_control_text;
  /**
   * @brief Expand control content when toggle expanded.
   */
  const std::string expand_collapse_content;
  /**
   * @brief Footer text.
   */
  const std::string footer_text;
  /**
   * @brief Buttons.
   */
  const DialogBoxButton buttons;
  /**
   * @brief Is Left to right text layout or not?
   */
  const bool rtl_layout;
  [[maybe_unused]] std::byte pad[sizeof(int) - sizeof(rtl_layout)];

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(DialogBoxSettings);
};

/**
 * @brief Shows dialog box.  The parent window should not be hidden or disabled
 * when this function is called.  Requires COM to be initialized before.
 * @param kind Dialog box kind.
 * @param settings Dialog box settings.
 * @return true on success, false on failure.
 */
WB_BASE_API std_ext::sc_res<DialogBoxButton> ShowDialogBox(
    DialogBoxKind kind, const DialogBoxSettings &settings) noexcept;
}  // namespace wb::base::windows::ui

#endif  // !WB_BASE_WINDOWS_UI_TASK_DIALOG_H_
