// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Peek message dispatcher.

#ifndef WB_BASE_INCLUDE_WINDOWS_UI_PEEK_MESSAGE_DISPATCHER_H_
#define WB_BASE_INCLUDE_WINDOWS_UI_PEEK_MESSAGE_DISPATCHER_H_
#ifdef _WIN32
#pragma once
#endif

#include <type_traits>

#include "base/include/base_macroses.h"
#include "base/include/deps/g3log/g3log.h"
#include "base/include/windows/windows_light.h"

namespace wb::base::windows::ui {
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

  // Dispatch messages to the window.  Not a blocking call, dispatch only
  // arrived in queue messages.
  template <
      typename HasPreDispatchedMessageFn = decltype(&HasNoPreDispatchMessage),
      typename PostDispatchedMessageFn = decltype(&HasNoPostDispatchMessage),
      typename =
          std::enable_if_t<std::is_nothrow_invocable_r_v<
                               bool, HasPreDispatchedMessageFn, const MSG&> &&
                           std::is_nothrow_invocable_r_v<
                               void, PostDispatchedMessageFn, const MSG&>>>
  void Dispatch(_In_ HasPreDispatchedMessageFn has_pre_dispatched_message_fn =
                    HasNoPreDispatchMessage,
                _In_ PostDispatchedMessageFn post_dispatched_message_fn =
                    HasNoPostDispatchMessage,
                _In_ UINT lowest_message_id = 0U,
                _In_ UINT highest_message_id = 0U) const noexcept {
    MSG msg;
    while (PeekMessage(&msg, hwnd_, lowest_message_id, highest_message_id,
                       PM_NOREMOVE)) {
      const BOOL rc{
          GetMessage(&msg, hwnd_, lowest_message_id, highest_message_id)};
      DCHECK(rc != -1);

      if (rc != -1 && !has_pre_dispatched_message_fn(msg)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        post_dispatched_message_fn(msg);
      } else {
        break;
      }
    }
  }

 private:
  // Handle to window which messages are dispatched.
  const HWND hwnd_;
};
}  // namespace wb::base::windows::ui

#endif  // !WB_BASE_INCLUDE_WINDOWS_UI_PEEK_MESSAGE_DISPATCHER_H_
