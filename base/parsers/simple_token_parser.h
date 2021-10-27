// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Simple token parser for small C subset.
//
// Formal syntax is (in: break-set, out: token-value-*):
// token_line:
//   whitespace-list(opt) cpp-comment
//   token-body
//
// whitespace-list:
//   whitespace
//   whitespace-list(opt)
//
// whitespace:
//   [any char with code <= ' ']{0, n}
//
// cpp-comment:
//   // comment-body new-line
//
// comment-body:
//   whitespace-list(opt)
//   any char with code > ' ' except new-line
//   comment-body(opt)
//
// new-line:
//   \n
//
// token-body:
//   " token-value1 "(opt)
//   token-value2
//   token-value3
//
// token-value1:
//   whitespace-list(opt)
//   any char with code > ' '
//   token-value1(opt)
//
// token-value2:
//   any character from break-set
//
// break-set:
//   any char
//   break-set(opt)
//
// token-value3:
//   any char with code > ' ' except one from break-set
//   token-value3(opt)

#ifndef WB_BASE_PARSERS_SIMPLE_TOKEN_PARSER_H_
#define WB_BASE_PARSERS_SIMPLE_TOKEN_PARSER_H_

#include <ostream>
#include <string>
#include <string_view>

#ifndef NDEBUG
#include "base/deps/g3log/g3log.h"
#endif

#include "base/parsers/character_set.h"

namespace wb::base::parsers::st {

/**
 * @brief Parsed token values.
 */
struct ParsedToken {
  // Next token to parse.  Empty if nothing to parse next.
  std::string_view next_token;
  // Current parsed token.  Empty if nothing was parsed (EOF, etc.).
  std::string_view current_token;
};

/**
 * @brief Operator == for ParsedToken.
 * @param l Left.
 * @param r Right.
 * @return true if equals, false otherwise.
 */
[[nodiscard]] constexpr bool operator==(const ParsedToken& l,
                                        const ParsedToken& r) noexcept {
  return l.current_token == r.current_token && l.next_token == r.next_token;
}

/**
 * @brief Operator != for ParsedToken.
 * @param l Left.
 * @param r Right.
 * @return false if equals, true otherwise.
 */
[[nodiscard]] constexpr bool operator!=(const ParsedToken& l,
                                        const ParsedToken& r) noexcept {
  return !(l == r);
}

/**
 * @brief Parse token from |data| using |breaks| character set.
 * @param data Data to parse token from.
 * @param breaks Character set is used as token breaks.
 * @return Parsed token.
 */
[[nodiscard]] constexpr ParsedToken ParseToken(std::string_view data,
                                               const CharacterSet& breaks) {
  if (data.empty()) {
    // Nothing to parse.
    return {.next_token = std::string_view{},
            .current_token = std::string_view{}};
  }

  size_t i{0};
  char c{'\0'};

  // Skip white spaces and comments.
  do {
    // Skip white spaces.
    while (i < data.size() && (c = data[i]) <= ' ') {
      i++;
    }

    if (i == data.size()) {
      // End of string is found, no token.
      return {.next_token = std::string_view{},
              .current_token = std::string_view{}};
    }

    // Skip // comments.
    if (c == '/' && i + 1 < data.size() && data[i + 1] == '/') {
      while (i < data.size() && data[i] != '\n') {
        i++;
      }
    } else {
      break;
    }
  } while (true);

  constexpr auto next_token = [](const std::string_view& d,
                                 size_t idx) noexcept -> std::string_view {
    return idx < d.size() ? d.substr(idx, d.size() - idx) : std::string_view{};
  };

  // Quoted token is extracted from quotes.
  if (c == '\"') {
    i++;

    const size_t token_start_idx{i};

    while (true) {
      if (i < data.size()) {
        c = data[i++];
      } else {
        return {
            .next_token = std::string_view{},
            .current_token = data.substr(token_start_idx, i - token_start_idx)};
      }

      if (c == '\"') {
        return {.next_token = next_token(data, i),
                .current_token =
                    data.substr(token_start_idx, i - 1 - token_start_idx)};
      }
    }
  }

  // Parse single break character.
  if (breaks.HasChar(c)) {
    const size_t token_start_idx{i};
    return {.next_token = next_token(data, i + 1),
            .current_token = data.substr(token_start_idx, 1)};
  }

  const size_t token_start_idx{i};

  // Parse word.
  do {
    if (++i < data.size()) {
      c = data[i];
    } else {
      break;
    }

    if (breaks.HasChar(c)) break;
  } while (c > ' ');

  return {.next_token = next_token(data, i),
          .current_token = data.substr(token_start_idx, i - token_start_idx)};
}

}  // namespace wb::base::parsers::st

#endif  // !WB_BASE_PARSERS_SIMPLE_TOKEN_PARSER_H_
