// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Peek window message queue dispatcher.

#ifndef WB_UI_WIN_PEEK_MESSAGE_DISPATCHER_H_
#define WB_UI_WIN_PEEK_MESSAGE_DISPATCHER_H_

#include <type_traits>

#include "base/macroses.h"
#include "base/deps/g3log/g3log.h"
#include "base/win/windows_light.h"

namespace wb::ui::win {

/**
 * @brief Default message predispatcher function.  Predispatch nothing.
 * @param Message.
 * @return false.
 */
constexpr inline bool HasNoPreDispatchMessage(const MSG&) noexcept {
  return false;
}

/**
 * @brief Default message postdispatcher function.  Postdispatch nothing.
 */
constexpr inline void HasNoPostDispatchMessage(const MSG&) noexcept {}

/**
 * @brief Dispatcher functions concept.
 * @tparam HasPreDispatchedMessageFn Predispatch function.
 * @tparam PostDispatchedMessageFn Postdispatch function.
 */
template <typename HasPreDispatchedMessage, typename PostDispatchedMessage>
using dispatcher_functions_concept = std::enable_if_t<
    std::is_nothrow_invocable_r_v<bool, HasPreDispatchedMessage, const MSG&> &&
    std::is_nothrow_invocable_r_v<void, PostDispatchedMessage, const MSG&>>;

/**
 * @brief Window messages peeking dispatcher.
 */
class PeekMessageDispatcher {
 public:
  /**
   * @brief Creates message dispatcher from handle to window which messages are
   * dispatched.
   */
  explicit PeekMessageDispatcher(_In_opt_ HWND hwnd = nullptr) noexcept
      : hwnd_{hwnd} {}

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(PeekMessageDispatcher);

  /**
   * @brief Dispatch messages to the window.  Not a blocking call, dispatch only
   * arrived in queue messages.
   * @tparam HasPreDispatchedMessageFn Message predispatcher.
   * @tparam PostDispatchedMessageFn Message postdispatcher.
   * @param has_pre_dispatched_message_fn Predispather.
   * @param post_dispatched_message_fn Postdispatcher.
   * @param lowest_message_id Lowest message id.  Ids lower than this are
   * ignored.
   * @param highest_message_id Highest message id.  Ids higher than this are
   * ignored.
   * @return void.
   */
  template <
      typename HasPreDispatchedMessage = decltype(&HasNoPreDispatchMessage),
      typename PostDispatchedMessage = decltype(&HasNoPostDispatchMessage)>
  dispatcher_functions_concept<HasPreDispatchedMessage, PostDispatchedMessage>
  Dispatch(_In_ HasPreDispatchedMessage has_pre_dispatched_message =
               HasNoPreDispatchMessage,
           _In_ PostDispatchedMessage post_dispatched_message =
               HasNoPostDispatchMessage,
           _In_ UINT lowest_message_id = 0U,
           _In_ UINT highest_message_id = 0U) const noexcept {
    MSG msg;
    while (PeekMessage(&msg, hwnd_, lowest_message_id, highest_message_id,
                       PM_NOREMOVE)) {
      const BOOL rc{
          GetMessage(&msg, hwnd_, lowest_message_id, highest_message_id)};
      G3DCHECK(rc != -1);

      if (rc != -1 && !has_pre_dispatched_message(msg)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        post_dispatched_message(msg);
      } else {
        break;
      }
    }
  }

 private:
  /**
   * @brief Handle to window which messages are dispatched.
   */
  const HWND hwnd_;
};

}  // namespace wb::ui::win

#endif  // !WB_UI_WIN_PEEK_MESSAGE_DISPATCHER_H_
