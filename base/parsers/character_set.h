// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Defines set of characters useful for parsers.

#ifndef WB_BASE_PARSERS_CHARACTER_SET_H_
#define WB_BASE_PARSERS_CHARACTER_SET_H_

#include <limits>
#include <string_view>

namespace wb::base::parsers {

/**
 * @brief Character set.  Used for parsers configuration.
 */
struct CharacterSet {
  using char_type = unsigned char;

  // +1 for [0, max()] range.
  bool set[std::numeric_limits<char_type>::max() + 1];

  /**
   * @brief Creates empty character set.
   * @return nothing.
   */
  constexpr CharacterSet() noexcept : set{false, false} {}

  /**
   * @brief Creates character set from |char_set|.
   * @param char_set
   * @return
   */
  explicit constexpr CharacterSet(std::string_view char_set) noexcept
      : CharacterSet{} {
    for (auto ch : char_set) {
      set[static_cast<char_type>(ch)] = true;
    }
  }

  /**
   * @brief Is character is character set?
   * @param ch Character to check in set.
   * @return true if |ch| belongs to character set, false otherwise.
   */
  [[nodiscard]] constexpr bool HasChar(const char ch) const noexcept {
    return set[static_cast<char_type>(ch)];
  }
};

}  // namespace wb::base::parsers

#endif  // !WB_BASE_PARSERS_CHARACTER_SET_H_