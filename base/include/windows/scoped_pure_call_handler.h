// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Scoped handler when pure virtual function call is detected.

#ifndef WB_BASE_INCLUDE_WINDOWS_SCOPED_PURE_CALL_HANDLER_H_
#define WB_BASE_INCLUDE_WINDOWS_SCOPED_PURE_CALL_HANDLER_H_
#ifdef _WIN32
#pragma once
#endif

#include <cstdlib>

#include "base/include/deps/g3log/g3log.h"

namespace wb::base::windows {
/**
 * @brief Sets a function to be called when pure virtual function call is
 * detected and reverts back when out of scope.
 */
class ScopedPureCallHandler {
 public:
  explicit ScopedPureCallHandler(
      _In_ _purecall_handler pure_call_handler) noexcept
      : old_pure_call_handler_{::_set_purecall_handler(pure_call_handler)} {
    DCHECK(!!pure_call_handler);
  }

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedPureCallHandler);

  ~ScopedPureCallHandler() noexcept {
    (void)::_set_purecall_handler(old_pure_call_handler_);
  }

 private:
  const _purecall_handler old_pure_call_handler_;
};

[[noreturn]] void DefaultPureCallHandler() {
  LOG(FATAL) << "Pure virtual function call occured.  Stopping the app.";
}
}  // namespace wb::base::windows

#endif  // !WB_BASE_INCLUDE_WINDOWS_SCOPED_PURE_CALL_HANDLER_H_
