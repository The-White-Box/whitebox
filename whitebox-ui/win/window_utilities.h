// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Common window utilities.

#ifndef WB_WHITEBOX_UI_WIN_WINDOW_UTILITIES_H_
#define WB_WHITEBOX_UI_WIN_WINDOW_UTILITIES_H_

#include <sal.h>

#include <chrono>
#include <cstdint>
#include <string>
#include <type_traits>

#include "whitebox-ui/config.h"

using HWND = struct HWND__ *;

namespace wb::ui::win {

/**
 * @brief Moves window to the center of its monitor.
 * @param hwnd Window.
 * @param repaint_after Should repaint window after move?
 * @return true on success, false otherwise.
 */
WB_WHITEBOX_UI_API bool MoveWindowToItsDisplayCenter(
    _In_ HWND window, _In_ bool repaint_after) noexcept;

/**
 * @brief Flashes window caption and title bar.
 * @param window_class_name Window class name.
 * @param timeout_between_flashes How many milliseconds to wait between window
 * flashes.
 * @return true if window is flashing, false otherwise.
 */
WB_WHITEBOX_UI_API bool FlashWindowByClass(
    _In_ const std::string &window_class_name,
    _In_ std::chrono::milliseconds timeout_between_flashes) noexcept;

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

}  // namespace wb::ui::win

#endif  // !WB_WHITEBOX_UI_WIN_WINDOW_UTILITIES_H_
