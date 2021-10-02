// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Character set for parsers.

#include "character_set.h"
//
#include "base/deps/googletest/gtest/gtest.h"

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(CharacterSetTest, DefaultConstructor) {
  using namespace wb::base::parsers;

  constexpr CharacterSet set;

  for (auto &&ch : set.set) {
    EXPECT_FALSE(ch);
  }
}

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(CharacterSetTest, SetConstructor) {
  using namespace wb::base::parsers;

  constexpr CharacterSet set{"123"};

  size_t idx{0};
  for (auto &&ch : set.set) {
    if (idx == static_cast<CharacterSet::char_type>('1') ||
        idx == static_cast<CharacterSet::char_type>('2') ||
        idx == static_cast<CharacterSet::char_type>('3')) {
      EXPECT_TRUE(ch) << "Char '"
                      << static_cast<char>(
                             static_cast<CharacterSet::char_type>(idx))
                      << " should be in set.";
    } else {
      EXPECT_FALSE(ch) << "Char '"
                       << static_cast<char>(
                              static_cast<CharacterSet::char_type>(idx))
                       << " should not be in set.";
    }
    ++idx;
  }
}

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(CharacterSetTest, HasChar) {
  using namespace wb::base::parsers;

  constexpr CharacterSet set{"{}()"};

  static_assert(set.HasChar('{'));
  static_assert(set.HasChar('}'));
  static_assert(set.HasChar('('));
  static_assert(set.HasChar(')'));

  static_assert(!set.HasChar(' '));
  static_assert(!set.HasChar('.'));
  static_assert(!set.HasChar('a'));
  static_assert(!set.HasChar('A'));
  static_assert(!set.HasChar('1'));
  static_assert(!set.HasChar('\\'));
  static_assert(!set.HasChar('\n'));
}