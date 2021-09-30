// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Common window utilities.

#ifndef WB_BASE_WIN_UI_WINDOW_UTILITIES_H_
#define WB_BASE_WIN_UI_WINDOW_UTILITIES_H_

#include <sal.h>

#include <cstdint>
#include <type_traits>

#include "base/base_api.h"

using HWND = struct HWND__ *;

namespace wb::base::windows::ui {
/**
 * @brief Moves window to the center of its monitor.
 * @param hwnd Window.
 * @param repaint_after Should repaint window after move?
 * @return true on success, false otherwise.
 */
WB_BASE_API bool MoveWindowToItsDisplayCenter(_In_ HWND window,
                                              _In_ bool repaint_after) noexcept;

/**
 * @brief Is resource id for MakeIntResource concept.
 * @tparam T Type to check.
 * @tparam R Type to return.
 */
template <typename T, typename R>
using is_resource_id_concept =
    std::enable_if_t<std::is_integral_v<T> && sizeof(T) <= sizeof(int), R>;

#ifdef UNICODE
/**
 * @brief Makes internal resource from |id|.
 * @tparam T Type of |id|.
 * @param id
 * @return Internal resource string.
 */
template <typename T>
[[nodiscard]] constexpr is_resource_id_concept<T, const wchar_t *>
MakeIntResource(T id) noexcept {
  return reinterpret_cast<const wchar_t *>(
      static_cast<std::uintptr_t>(static_cast<unsigned short>(id)));
}
#else
/**
 * @brief Makes internal resource from |id|.
 * @tparam T Type of |id|.
 * @param id
 * @return Internal resource string.
 */
template <typename T>
[[nodiscard]] constexpr is_resource_id_concept<T, const char *> MakeIntResource(
    T id) noexcept {
  return reinterpret_cast<const char *>(
      static_cast<std::uintptr_t>(static_cast<unsigned short>(id)));
}
#endif  // !UNICODE
}  // namespace wb::base::windows::ui

#endif  // !WB_BASE_WIN_UI_WINDOW_UTILITIES_H_
