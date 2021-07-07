// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Scoped window class.

#ifndef WB_BASE_WINDOWS_UI_SCOPED_WINDOW_CLASS_H_
#define WB_BASE_WINDOWS_UI_SCOPED_WINDOW_CLASS_H_

#include "base/base_macroses.h"
#include "base/deps/g3log/g3log.h"
#include "base/windows/system_error_ext.h"
#include "base/windows/windows_light.h"

namespace wb::base::windows::ui {
/**
 * @brief Registers window class in scope.
 */
class ScopedWindowClass {
 public:
  /**
   * @brief Creates scoped window class.
   * @param class_definition Window class definition.
   * @return nothing.
   */
  explicit ScopedWindowClass(_In_ WNDCLASSEX &class_definition) noexcept
      : instance_{class_definition.hInstance},
        class_atom_{::RegisterClassEx(&class_definition)},
        error_code_{GetErrorCode(class_atom_)} {
    G3DCHECK(!error_code());
  }

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedWindowClass);

  /**
   * @brief Free window class.
   */
  ~ScopedWindowClass() noexcept {
    if (!error_code_) {
      const std::error_code rc{GetErrorCode(::UnregisterClass(
          reinterpret_cast<LPCTSTR>(
              static_cast<DWORD_PTR>(MAKELONG(class_atom_, 0))),
          instance_))};
      G3CHECK(!rc);
    }
  }

  /**
   * @brief Get class registration result.
   * @return Error code for clas registration result.
   */
  [[nodiscard]] std::error_code error_code() const noexcept {
    return error_code_;
  }

 private:
  /**
   * @brief App instance.
   */
  const HINSTANCE instance_;
  /**
   * @brief Class atom.
   */
  const ATOM class_atom_;
  [[maybe_unused]] std::byte pad_[sizeof(char *) - sizeof(class_atom_)];
  const std::error_code error_code_;
};
}  // namespace wb::base::windows::ui

#endif  // !WB_BASE_WINDOWS_UI_SCOPED_WINDOW_CLASS_H_
