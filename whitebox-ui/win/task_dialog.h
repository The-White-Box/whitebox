// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Windows Task Dialog.

#ifndef WB_WHITEBOX_UI_WIN_TASK_DIALOG_H_
#define WB_WHITEBOX_UI_WIN_TASK_DIALOG_H_

#include <sal.h>

#include <cstddef>  // std::byte
#include <optional>
#include <string>
#include <variant>

#include "base/base_macroses.h"
#include "base/std2/system_error_ext.h"
#include "whitebox-ui/api.h"

using HWND = struct HWND__ *;

namespace wb::ui::win {

/**
 * @brief Kind of dialog box.
 */
enum class DialogBoxKind {
  /**
   * @brief Information.
   */
  kInformation,
  /**
   * @brief Warning.
   */
  kWarning,
  /**
   * @brief Error.
   */
  kError,
  /**
   * @brief Shield.
   */
  kShield
};

/**
 * @brief Dialog box buttons.  Should be same as TASKDIALOG_COMMON_BUTTON_FLAGS.
 */
enum class DialogBoxButton {
  /**
   * @brief Ok.
   */
  kOk = 0x01,
  /**
   * @brief Yes
   */
  kYes = 0x02,
  /**
   * @brief No
   */
  kNo = 0x04,
  /**
   * @brief Cancel.
   */
  kCancel = 0x08,
  /**
   * @brief Retry.
   */
  kRetry = 0x10,
  /**
   * @brief Close.
   */
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
  return static_cast<DialogBoxButton>(base::underlying_cast(left) |
                                      base::underlying_cast(right));
}

/**
 * @brief operator& for DialogBoxButton.
 * @param left Left.
 * @param right Right.
 * @return Left & Right.
 */
[[nodiscard]] constexpr DialogBoxButton operator&(
    DialogBoxButton left, DialogBoxButton right) noexcept {
  return static_cast<DialogBoxButton>(base::underlying_cast(left) &
                                      base::underlying_cast(right));
}

/**
 * @brief Collapse control settings.
 */
struct DialogBoxCollapseSettings {
  DialogBoxCollapseSettings(
      const std::string &expanded_control_text_,
      const std::string &collapsed_control_text_,
      const std::string &expand_collapse_content_) noexcept
      : expanded_control_text{expanded_control_text_},
        collapsed_control_text{collapsed_control_text_},
        expand_collapse_content{expand_collapse_content_} {}

  /**
   * @brief Expanded toggle control text.
   */
  const std::string &expanded_control_text;
  /**
   * @brief Collapsed toggle control text.
   */
  const std::string &collapsed_control_text;
  /**
   * @brief Expand control content when toggle expanded.
   */
  const std::string &expand_collapse_content;

  DialogBoxCollapseSettings(DialogBoxCollapseSettings &s) noexcept = default;
  DialogBoxCollapseSettings(DialogBoxCollapseSettings &&s) noexcept = default;
  DialogBoxCollapseSettings &operator=(DialogBoxCollapseSettings &s) noexcept =
      delete;
  DialogBoxCollapseSettings &operator=(DialogBoxCollapseSettings &&s) noexcept =
      delete;
};

/**
 * @brief Dialog box settings.
 */
struct DialogBoxSettings {
  /**
   * @brief Creates dialog box settings.
   * @param parent_window_ Parent window.  May be nullptr.
   * @param title_ Dialog box title.
   * @param main_instruction_ Main instruction.
   * @param collapse_settings_ Collapse control settings.
   * @param content_ Content.
   * @param footer_text_ Footer text.
   * @param rtl_layout_ Is Left to right text layout or not?
   * @param is_cancellable_ Is cancel button available?
   * @return nothing.
   */
  DialogBoxSettings(_In_opt_ HWND parent_window_, const std::string &title_,
                    const std::string &main_instruction_,
                    std::optional<DialogBoxCollapseSettings> collapse_settings_,
                    const std::string &content_,
                    const std::string &footer_text_, DialogBoxButton buttons_,
                    int main_icon_id_, int small_icon_id_,
                    bool rtl_layout_) noexcept
      : parent_window{parent_window_},
        title{title_},
        main_instruction{main_instruction_},
        collapse_settings{std::move(collapse_settings_)},
        content{content_},
        footer_text{footer_text_},
        buttons{buttons_},
        main_icon_id{main_icon_id_},
        small_icon_id{small_icon_id_},
        rtl_layout{rtl_layout_} {}

  /**
   * @brief Parent window.  Optional.
   */
  const HWND parent_window;
  /**
   * @brief Title.
   */
  const std::string &title;
  /**
   * @brief Main instruction.
   */
  const std::string &main_instruction;
  /**
   * @brief Collapse control settings.
   */
  const std::optional<DialogBoxCollapseSettings> collapse_settings;
  /**
   * @brief Content.
   */
  const std::string &content;
  /**
   * @brief Footer text.
   */
  const std::string &footer_text;
  /**
   * @brief Buttons.
   */
  const DialogBoxButton buttons;
  /**
   * @brief Main icon id.
   */
  const int main_icon_id;
  /**
   * @brief Small icon id.
   */
  const int small_icon_id;
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
WB_WHITEBOX_UI_API base::std2::result<DialogBoxButton> ShowDialogBox(
    DialogBoxKind kind, const DialogBoxSettings &settings) noexcept;

}  // namespace wb::ui::win

#endif  // !WB_WHITEBOX_UI_WIN_TASK_DIALOG_H_
