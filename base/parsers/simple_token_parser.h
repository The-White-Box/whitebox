// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Simple token parser for small C subset.
//
// Format syntax is (in: break-set, out: token-value-*):
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
  std::string current_token;
};

/**
 * @brief Operator == for ParsedToken.
 * @param l Left.
 * @param r Right.
 * @return true if equals, false otherwise.
 */
[[nodiscard]] inline bool operator==(const ParsedToken& l,
                                     const ParsedToken& r) noexcept {
  return l.current_token == r.current_token && l.next_token == r.next_token;
}

/**
 * @brief Operator != for ParsedToken.
 * @param l Left.
 * @param r Right.
 * @return false if equals, true otherwise.
 */
[[nodiscard]] inline bool operator!=(const ParsedToken& l,
                                     const ParsedToken& r) noexcept {
  return !(l == r);
}

/**
 * @brief Parse token from |data| using |breaks| character set.
 * @param data Data to parse token from.
 * @param breaks Character set is used as token breaks.
 * @param estimated_max_token_size Estimated maximum token size.
 * @return Parsed token.
 */
[[nodiscard]] constexpr ParsedToken ParseToken(
    std::string_view data, const CharacterSet& breaks,
    size_t estimated_max_token_size = 1024U) {
  std::string token;

  if (data.empty()) {
    // Nothing to parse.
    return {.next_token = std::string_view{}, .current_token = token};
  }

  size_t i{0};
  char c{'\0'};
  token.reserve(estimated_max_token_size);

#ifndef NDEBUG
  const size_t initial_token_capacity{token.capacity()};
  const auto verify_and_get_token = [initial_token_capacity](
                                        std::string tn) noexcept {
    if (tn.size() > initial_token_capacity) WB_ATTRIBUTE_UNLIKELY {
        G3LOG(WARNING) << "Token uses more space (" << tn.size()
                       << ") than initial capacity (" << initial_token_capacity
                       << ").  Performance is degraded due to token "
                          "reallocations + copying.";
      }
    return tn;
  };
#else   // NDEBUG
  const auto verify_and_get_token = [](std::string tn) noexcept { return tn; };
#endif  // !NDEBUG

  // Skip white spaces and comments.
  do {
    // Skip white spaces.
    while (i < data.size() && (c = data[i]) <= ' ') {
      i++;
    }

    if (i == data.size()) {
      // End of string is found, time to return token.
      return {.next_token = std::string_view{},
              .current_token = verify_and_get_token(token)};
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

    while (true) {
      if (i < data.size()) {
        c = data[i++];
      } else {
        return {.next_token = std::string_view{},
                .current_token = verify_and_get_token(token)};
      }

      if (c == '\"') {
        return {.next_token = next_token(data, i),
                .current_token = verify_and_get_token(token)};
      }

      token.append(1, c);
    }
  }

  // Parse single break character.
  if (breaks.HasChar(c)) {
    token.append(1, c);
    return {.next_token = next_token(data, i + 1),
            .current_token = verify_and_get_token(token)};
  }

  // Parse word.
  do {
    token.append(1, c);

    if (++i < data.size()) {
      c = data[i];
    } else {
      break;
    }

    if (breaks.HasChar(c)) break;
  } while (c > ' ');

  return {.next_token = next_token(data, i),
          .current_token = verify_and_get_token(token)};
}

}  // namespace wb::base::parsers::st

#endif  // !WB_BASE_PARSERS_SIMPLE_TOKEN_PARSER_H_
