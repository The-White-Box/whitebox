// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// <type_traits> extensions.

#ifndef WB_BASE_STD_TYPE_TRAITS_EXT_H_
#define WB_BASE_STD_TYPE_TRAITS_EXT_H_

#include <type_traits>

namespace wb::base::std2 {
/**
 * @brief Is argument pointer to a function?
 * @tparam T Argument.
 */
template <typename T>
struct is_function_pointer {
  /**
   * @brief Is pointer to a function?
   */
  static constexpr bool value{
      std::is_pointer_v<T> &&
      std::is_function_v<typename std::remove_pointer_t<T>>};
};

/**
 * @brief Alias for is_function_pointer<T>::value.
 * @tparam T Argument.
 */
template <typename T>
constexpr bool is_function_pointer_v{is_function_pointer<T>::value};

/**
 * @brief Checks argument is char type (char, wchar_t, char16_t, char32_t).
 * @tparam T Argument.
 */
template <typename T>
struct is_char {
  /**
   * @brief Is char type?
   */
  static constexpr bool value{std::is_same_v<std::remove_cv_t<T>, char> ||
                              std::is_same_v<std::remove_cv_t<T>, wchar_t> ||
                              std::is_same_v<std::remove_cv_t<T>, char8_t> ||
                              std::is_same_v<std::remove_cv_t<T>, char16_t> ||
                              std::is_same_v<std::remove_cv_t<T>, char32_t>};
};

/**
 * @brief Alias for is_char<T>::value.
 * @tparam T Argument.
 */
template <typename T>
constexpr bool is_char_v{is_char<T>::value};
}  // namespace wb::base::std2

#endif  // !WB_BASE_STD_TYPE_TRAITS_EXT_H_
