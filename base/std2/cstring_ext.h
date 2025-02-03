// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// <cstring> extensions.

#ifndef WB_BASE_STD2_CSTRING_EXT_H_
#define WB_BASE_STD2_CSTRING_EXT_H_

#include <cstring>
#include <string>
#include <type_traits>

#include "base/config.h"
#include "build/build_config.h"

namespace wb::base::std2 {

/**
 * @brief Writtable memory concept.
 * @tparam TMemory Type to check as writeable memory.
 */
template <typename TMemory, std::size_t count>
concept writable_memory =
    std::is_trivially_copyable_v<TMemory> && count <= sizeof(TMemory);

/**
 * @brief std::memset with type-checking.
 * @tparam Dest Destination object.
 * @param destination Destination.
 * @param value Byte value to fill destination.
 * @return Destination object reference.
 */
template <typename Dest, std::size_t count = sizeof(Dest)>
  requires writable_memory<Dest, count>
inline Dest& BitwiseMemset(Dest& destination, unsigned char value) noexcept {
  return *static_cast<Dest*>(std::memset(&destination, value, count));
}

/**
 * @brief std::memcmp with type-checking.
 * @tparam Left Left.
 * @tparam Right Right.
 * @param left Left.
 * @param right Right.
 * @return < 0 if Left less than Right;  0 if Left identical to Right;  > 0 if
 * Left greater than Right.
 */
template <typename Left, typename Right, std::size_t size = sizeof(Left)>
[[nodiscard]] inline int BitwiseCompare(const Left& left,
                                        const Right& right) noexcept {
  static_assert(sizeof(left) >= size, "Verify left has at least size bytes.");
  static_assert(sizeof(right) >= size, "Verify right has at least size bytes.");

  return std::memcmp(&left, &right, size);
}

/**
 * @brief Bitwise std::memcpy with type-checking.
 * @tparam Dest Dest type.
 * @tparam Source Source type.
 * @param dest Dest.
 * @param source Source.
 * @return Reference to dest.
 */
template <typename Dest, typename Source>
inline Dest& BitwiseCopy(Dest& dest, const Source& source) noexcept {
  static_assert(sizeof(Dest) >= sizeof(Source), "Verify Dest size >= Source.");
  static_assert(std::is_trivially_copyable_v<Source>,
                "Verify Source should be trivially copyable.");
  static_assert(std::is_trivially_copyable_v<Dest>,
                "Verify Dest be trivially copyable.");
  static_assert(std::is_default_constructible_v<Dest>,
                "Verify Dest should be default constructible.");

  return *static_cast<Dest*>(std::memcpy(&dest, &source, sizeof(dest)));
}

}  // namespace wb::base::std2

#endif  // !WB_BASE_STD2_CSTRING_EXT_H_
