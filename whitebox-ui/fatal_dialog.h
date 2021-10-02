// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.

#ifndef WB_WHITEBOX_UI_FATAL_DIALOG_H_
#define WB_WHITEBOX_UI_FATAL_DIALOG_H_

#include <cstddef>
#include <optional>
#include <system_error>

#include "base/base_macroses.h"
#include "base/intl/lookup.h"
#include "whitebox-ui/api.h"

namespace wb::ui {

/**
 * @brief Special os context for fatal dialog.
 */
struct FatalDialogContext {
#ifdef WB_OS_WIN
  FatalDialogContext(const base::intl::LookupWithFallback& intl_,
                     const base::intl::StringLayout text_layout_,
                     const int main_icon_id_, const int small_icon_id_) noexcept
      : intl{intl_},
        text_layout{text_layout_},
        main_icon_id{main_icon_id_},
        small_icon_id{small_icon_id_} {}

  /**
   * @brief Localization service.
   */
  const base::intl::LookupWithFallback& intl;
  /**
   * Dialog content layout.
   */
  const base::intl::StringLayout text_layout;
  /**
   * @brief Main icon id to use in fatal dialog.
   */
  const int main_icon_id;
  /**
   * @brief Small icon id to use in fatal dialog.
   */
  const int small_icon_id;

  std::byte pad[4];
#else
  FatalDialogContext(const base::intl::StringLayout text_layout_) noexcept
      : text_layout{text_layout_} {}

  /**
   * Dialog content layout.
   */
  const base::intl::StringLayout text_layout;
#endif

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(FatalDialogContext);
};

/**
 * @brief Shows fatal dialog and exits.
 * @param title Title.
 * @param rc Error code for technical details.
 * @param main_instruction_message Main instruction message.
 * @param context OS specific context.
 * @param content_message Content message.
 * @return void.
 */
[[noreturn]] WB_WHITEBOX_UI_API WB_ATTRIBUTE_COLD void FatalDialog(
    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    const std::string& title, std::optional<std::error_code> rc,
    const std::string& main_instruction_message,
    const FatalDialogContext& context,
    const std::string& content_message) noexcept;

}  // namespace wb::ui

#endif  // !WB_WHITEBOX_UI_FATAL_DIALOG_H_
