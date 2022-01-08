// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Fatal dialog.  Shows OS-specific UI dialog and exits the app.

#include "fatal_dialog.h"

#include <algorithm>
#include <array>
#include <cstdlib>

#ifdef WB_OS_POSIX
#include "base/deps/sdl/message_box.h"
#endif

#ifdef WB_OS_WIN
#include "base/intl/l18n.h"
#include "ui/win/task_dialog.h"
#endif

#include "base/deps/g3log/g3log.h"
#include "base/std2/string_view_ext.h"
#include "build/compiler_config.h"

namespace {

#ifdef WB_OS_POSIX
/**
 * Adds end sentence mark to message.
 * @param error_message Error message.
 * @return Error message with end sentence mark.
 */
[[nodiscard]] WB_ATTRIBUTE_CONST std::string HumanizeMessageSentence(
    std::string&& error_message) noexcept {
  constexpr std::array<const char*, 6> end_sentence_marks{
      {".", "?", "!", "\r", "\n", "\r\n"}};

  const bool already_has_end_sentence_mark{
      std::find_if(end_sentence_marks.begin(), end_sentence_marks.end(),
                   [&error_message](const char* mark) noexcept {
                     return wb::base::std2::ends_with(error_message, mark);
                   }) != end_sentence_marks.end()};

  return already_has_end_sentence_mark ? std::move(error_message)
                                       : (error_message += '.');
}
#endif

}  // namespace

namespace wb::ui {

[[nodiscard]] WB_WHITEBOX_UI_API WB_ATTRIBUTE_COLD int FatalDialog(
    const std::string& title, std::optional<std::error_code> rc,
    const std::string& main_instruction_message,
    const FatalDialogContext& context,
    const std::string& content_message) noexcept {
  try {
#ifdef WB_OS_POSIX
    std::string error_message{main_instruction_message};
    error_message += "\n\n";
    error_message += content_message;
#endif

    if (rc.has_value()) {
      const auto errc = *rc;
#ifdef WB_OS_POSIX
      error_message += "\n\n";
      error_message += HumanizeMessageSentence(errc.message());
#endif

      G3PLOG_E(WARNING, errc)
          << main_instruction_message << "  " << content_message;
    } else {
      G3LOG(WARNING) << main_instruction_message << "  " << content_message;
    }

#ifdef WB_OS_POSIX
    const sdl::MessageBoxFlags flags{
        context.text_layout == base::intl::StringLayout::LeftToRight
            ? sdl::MessageBoxFlags::LeftToRight
            : sdl::MessageBoxFlags::RightToLeft};
    [[maybe_unused]] const auto error =
        sdl::ShowSimpleMessageBox(flags | sdl::MessageBoxFlags::Error,
                                  title.c_str(), error_message.c_str());
    // Well, dialog may not be shown (too low RAM, etc.).  So just ignore result
    // in Release.
    G3DCHECK(error.is_succeeded()) << "Fatal dialog can't be shown: " << error;
#elif defined(WB_OS_WIN)
    using namespace wb::base;
    using namespace wb::ui::win;

    auto& intl = context.intl;

    const std::string technical_details{rc.has_value() ? rc.value().message()
                                                       : ""};
    auto collapse_settings =
        !technical_details.empty()
            ? std::optional<DialogBoxCollapseSettings>(
                  std::in_place, intl::l18n(intl, "Hide techical details"),
                  intl::l18n(intl, "See techical details"), technical_details)
            : std::optional<DialogBoxCollapseSettings>{};
    const bool rtl_layout{context.text_layout ==
                          intl::StringLayout::RightToLeft};

    DialogBoxSettings dialog_settings(
        nullptr, title, main_instruction_message, std::move(collapse_settings),
        content_message,
        intl::l18n(intl,
                   "<A "
                   "HREF=\"https://github.com/The-White-Box/whitebox/"
                   "issues\">Nudge</A> authors"),
        DialogBoxButton::kOk, context.main_icon_id, context.small_icon_id,
        rtl_layout);
    [[maybe_unused]] const auto result =
        ShowDialogBox(DialogBoxKind::kError, dialog_settings);
    // Well, dialog may not be shown (too low RAM, etc.).  So just ignore result
    // in Release.
    G3DCHECK(!std2::get_error(result))
        << "Fatal dialog can't be shown: " << *std2::get_error(result);
#else
#error "Please define FatalDialog UI for your platform."
#endif
  } catch (const std::exception& ex) {
    G3LOG(WARNING) << "Exception caught in FatalDialog: " << ex.what();
  }

  const auto exit_code =
      rc.value_or(std::error_code{-1, std::generic_category()});
  return exit_code.value();
}

}  // namespace wb::ui
