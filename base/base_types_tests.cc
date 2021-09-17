// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Small and precise synonyms of fixed width standard types.  These aliases are
// not defined if no types with such characteristics exist, so define for your
// platform as needed.

#include "base_types.h"
//
#include <type_traits>
//
#include "base/deps/googletest/gtest/gtest.h"

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(BaseTypesTest, FixedWidthTypes) {
  static_assert(sizeof(wb::u8) == 1);
  static_assert(sizeof(wb::i8) == 1);
  static_assert(sizeof(wb::u16) == 2);
  static_assert(sizeof(wb::i16) == 2);
  static_assert(sizeof(wb::u32) == 4);
  static_assert(sizeof(wb::i32) == 4);
  static_assert(sizeof(wb::u64) == 8);
  static_assert(sizeof(wb::i64) == 8);
  static_assert(sizeof(wb::usize) == sizeof(char*));
  static_assert(sizeof(wb::isize) == sizeof(char*));
}

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(BaseMacrosesTest, FixedWidthTypesSign) {
  static_assert(std::is_unsigned_v<wb::u8>);
  static_assert(std::is_signed_v<wb::i8>);
  static_assert(std::is_unsigned_v<wb::u16>);
  static_assert(std::is_signed_v<wb::i16>);
  static_assert(std::is_unsigned_v<wb::u32>);
  static_assert(std::is_signed_v<wb::i32>);
  static_assert(std::is_unsigned_v<wb::u64>);
  static_assert(std::is_signed_v<wb::i64>);
  static_assert(std::is_unsigned_v<wb::usize>);
  static_assert(std::is_signed_v<wb::isize>);
}