// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Character set for parsers.

#include "simple_token_parser.h"
//
#include <array>
#include <tuple>
//
#include "base/deps/googletest/gtest/gtest.h"

namespace wb::base::parsers::st {

/**
 * @brief Allows to use operator << for streaming ParsedToken.
 * @param s Stream.
 * @param parsed_token ParsedToken.
 * @return Stream.
 */
inline auto& operator<<(
    std::ostream& s, const wb::base::parsers::st::ParsedToken& parsed_token) {
  return s << "{current: '" << parsed_token.current_token << "', next: '"
           << parsed_token.next_token << "\'}";
}

}  // namespace wb::base::parsers::st

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(SimpleTokenParserTest, ParseToken) {
  using namespace wb::base::parsers;
  using namespace wb::base::parsers::st;

  // wordbreak parsing set
  constexpr CharacterSet break_set{"{}()'"};

  std::array<std::tuple<std::string_view, ParsedToken>, 62> cases{
      std::tuple{"", ParsedToken{}},
      std::tuple{" ", ParsedToken{}},
      std::tuple{"  ", ParsedToken{}},
      std::tuple{" a", ParsedToken{.current_token = "a"}},
      std::tuple{" 1 ", ParsedToken{.next_token = " ", .current_token = "1"}},
      std::tuple{"  c ", ParsedToken{.next_token = " ", .current_token = "c"}},
      std::tuple{"  2  ",
                 ParsedToken{.next_token = "  ", .current_token = "2"}},
      std::tuple{"//", ParsedToken{}},
      std::tuple{"/", ParsedToken{.current_token = "/"}},
      std::tuple{"/ ", ParsedToken{.next_token = " ", .current_token = "/"}},
      std::tuple{"/ a", ParsedToken{.next_token = " a", .current_token = "/"}},
      std::tuple{"// ", ParsedToken{}},
      std::tuple{"//  ", ParsedToken{}},
      std::tuple{"//d", ParsedToken{}},
      std::tuple{"//ef", ParsedToken{}},
      std::tuple{"// 3", ParsedToken{}},
      std::tuple{"// 45", ParsedToken{}},
      std::tuple{"// g ", ParsedToken{}},
      std::tuple{"// gh ", ParsedToken{}},
      std::tuple{"// i  ", ParsedToken{}},
      std::tuple{"// jk  ", ParsedToken{}},
      std::tuple{"//  4", ParsedToken{}},
      std::tuple{"//  a ", ParsedToken{}},
      std::tuple{"//  a  \nb", ParsedToken{.current_token = "b"}},
      std::tuple{"//  a  \nb ", ParsedToken{.next_token = " ", .current_token = "b"}},
      std::tuple{"//  a  \nbc d", ParsedToken{.next_token = " d", .current_token = "bc"}},
      std::tuple{"//  a  \nbc d\n", ParsedToken{.next_token = " d\n", .current_token = "bc"}},
      std::tuple{"//  a  \n{bc", ParsedToken{.next_token = "bc", .current_token = "{"}},
      std::tuple{" /", ParsedToken{.current_token = "/"}},
      std::tuple{" /a", ParsedToken{.current_token = "/a"}},
      std::tuple{" /ac", ParsedToken{.current_token = "/ac"}},
      std::tuple{" /de/", ParsedToken{.current_token = "/de/"}},
      // Invalid case.
      std::tuple{" /fg/\"", ParsedToken{.current_token = "/fg/\""}},
      std::tuple{" /eh/ \"",
                 ParsedToken{.next_token = " \"", .current_token = "/eh/"}},
      std::tuple{" / a", ParsedToken{.next_token = " a", .current_token = "/"}},
      std::tuple{" / a /",
                 ParsedToken{.next_token = " a /", .current_token = "/"}},
      std::tuple{" //", ParsedToken{}},
      std::tuple{" //\"a\"", ParsedToken{}},
      std::tuple{" //a", ParsedToken{}},
      std::tuple{" // b", ParsedToken{}},
      std::tuple{"  //", ParsedToken{}},
      std::tuple{"  // c", ParsedToken{}},
      std::tuple{"  // d ", ParsedToken{}},
      std::tuple{"/ /", ParsedToken{.next_token = " /", .current_token = "/"}},
      std::tuple{"/ /a",
                 ParsedToken{.next_token = " /a", .current_token = "/"}},
      std::tuple{"\"", ParsedToken{}},
      std::tuple{"\"\"", ParsedToken{}},
      std::tuple{"\"a", ParsedToken{.current_token = "a"}},
      std::tuple{"\" a", ParsedToken{.current_token = " a"}},
      std::tuple{"\" a\"", ParsedToken{.current_token = " a"}},
      std::tuple{"\" a \"", ParsedToken{.current_token = " a "}},
      std::tuple{"\"  a  \"", ParsedToken{.current_token = "  a  "}},
      std::tuple{"\"  a  \"{",
                 ParsedToken{.next_token = "{", .current_token = "  a  "}},
      // Invalid case.
      std::tuple{"a\"b", ParsedToken{.current_token = "a\"b"}},
      std::tuple{"a \"c",
                 ParsedToken{.next_token = " \"c", .current_token = "a"}},
      std::tuple{"{abc}",
                 ParsedToken{.next_token = "abc}", .current_token = "{"}},
      std::tuple{" } abc",
                 ParsedToken{.next_token = " abc", .current_token = "}"}},
      std::tuple{"// {abc}", ParsedToken{}},
      std::tuple{" (123)",
                 ParsedToken{.next_token = "123)", .current_token = "("}},
      std::tuple{")45", ParsedToken{.next_token = "45", .current_token = ")"}},
      std::tuple{" abc) ",
                 ParsedToken{.next_token = ") ", .current_token = "abc"}},
      std::tuple{" d ef ) ",
                 ParsedToken{.next_token = " ef ) ", .current_token = "d"}},
  };

  for (const auto& c : cases) {
    auto [input, token] = c;

    EXPECT_EQ(ParseToken(input, break_set), token)
        << "Should parse '" << input << "'";
  }
}