// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Small and precise synonyms of fixed width standard types.  These aliases are
// not defined if no types with such characteristics exist, so define for your
// platform as needed.

#ifndef WB_BASE_BASE_TYPES_H_
#define WB_BASE_BASE_TYPES_H_

#include <cstddef>  // std::size_t, std::ptrdiff_t.
#include <cstdint>  // precise bit integral types.

namespace wb {

// uint8_t.
using u8 = std::uint8_t;
// int8_t.
using i8 = std::int8_t;

// uint16_t.
using u16 = std::uint16_t;
// int16_t.
using i16 = std::int16_t;

// uint32_t.
using u32 = std::uint32_t;
// int32_t.
using i32 = std::int32_t;

// uint64_t.
using u64 = std::uint64_t;
// int64_t.
using i64 = std::int64_t;

// Max array index size type.
using usize = std::size_t;
// Signed pointer difference size type.
using isize = std::ptrdiff_t;

// 4 bytes float.
using f32 = float;
static_assert(sizeof(f32) == 4,  //-V112
              "f32 should be 4 bytes length. "
              "Please, define 4 bytes float for your platform.");

// 8 bytes double.
using f64 = double;
static_assert(sizeof(f64) == 8,
              "f64 should be 8 bytes length. "
              "Please, define 8 bytes double for your platform.");

// char.
using ch = char;
static_assert(sizeof(ch) == 1,
              "char should be 1 byte length. "
              "Please, define 1 byte char for your platform.");

// char16_t.
using ch16 = char16_t;
// char32_t.
using ch32 = char32_t;
// wchar_t.
using wch = wchar_t;

}  // namespace wb

#endif  // !WB_BASE_BASE_TYPES_H_
