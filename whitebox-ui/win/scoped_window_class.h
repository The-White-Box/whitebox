// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Scoped window class.

#ifndef WB_WHITEBOX_UI_WIN_SCOPED_WINDOW_CLASS_H_
#define WB_WHITEBOX_UI_WIN_SCOPED_WINDOW_CLASS_H_

#include <sal.h>

#include <array>
#include <cstdint>

#include "base/base_macroses.h"
#include "base/deps/g3log/g3log.h"
#include "base/win/system_error_ext.h"
#include "build/compiler_config.h"

using WNDCLASSEXA = struct tagWNDCLASSEXA;
using HINSTANCE = struct HINSTANCE__ *;

extern "C" WB_ATTRIBUTE_DLL_IMPORT unsigned short __stdcall RegisterClassExA(
    _In_ const WNDCLASSEXA *);
extern "C" WB_ATTRIBUTE_DLL_IMPORT int __stdcall UnregisterClassA(
    _In_ const char *lpClassName, _In_opt_ HINSTANCE hInstance);

namespace wb::ui::win {

/**
 * @brief Make long from args.
 * @param a Left.
 * @param b Right.
 * @return Long.
 */
[[nodiscard]] constexpr long MakeLong(auto a, auto b) noexcept {
  return (long)(((unsigned short)(((uintptr_t)(a)) & 0xffff)) |
                ((unsigned long)((unsigned short)(((uintptr_t)(b)) & 0xffff)))
                    << 16);
}

/**
 * @brief Registers window class in scope.
 */
class ScopedWindowClass {
 public:
  /**
   * @brief Creates scoped window class.
   * @param instance App instance.
   * @param class_definition Window class definition.
   * @return nothing.
   */
  explicit ScopedWindowClass(_In_ HINSTANCE instance,
                             _In_ WNDCLASSEXA &class_definition) noexcept
      : instance_{instance},
        class_atom_{::RegisterClassExA(&class_definition)},
        error_code_{base::win::get_error(class_atom_)} {
    G3DCHECK(!error_code());
  }

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedWindowClass);

  /**
   * @brief Free window class.
   */
  ~ScopedWindowClass() noexcept {
    if (!error_code_) {
      const std::error_code rc{base::win::get_error(::UnregisterClassA(
          reinterpret_cast<const char *>(
              static_cast<uintptr_t>(MakeLong(class_atom_, 0))),
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
  const unsigned short class_atom_;

  [[maybe_unused]] std::array<std::byte, sizeof(char *) - sizeof(class_atom_)>
      pad_;

  /**
   * @brief Error code.
   */
  const std::error_code error_code_;
};

}  // namespace wb::ui::win

#endif  // !WB_WHITEBOX_UI_WIN_SCOPED_WINDOW_CLASS_H_
