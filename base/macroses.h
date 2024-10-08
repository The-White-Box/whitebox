// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Provides common macroses & inline functions.

#ifndef WB_BASE_MACROSES_H_
#define WB_BASE_MACROSES_H_

#include <memory>
#include <utility>

#include "base/policy_checks.h"

// clang-format off
/**
 * Deletes copy ctor and assignment operator for type.
 */
#define WB_NO_COPY_CTOR_AND_ASSIGNMENT(typeName) \
  typeName(typeName&) = delete;                  \
  typeName& operator=(typeName&) = delete

/**
 * Deletes move ctor and assignment operator for type.
 */
#define WB_NO_MOVE_CTOR_AND_ASSIGNMENT(typeName) \
  typeName(typeName&&) = delete;                 \
  typeName& operator=(typeName&&) = delete
// clang-format on

/**
 * Deletes copy / move ctors and assignment operators for type.
 */
#define WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(typeName) \
  WB_NO_COPY_CTOR_AND_ASSIGNMENT(typeName);           \
  WB_NO_MOVE_CTOR_AND_ASSIGNMENT(typeName)

namespace wb::base {

/**
 * @brief Implicit, checked at compile time cast.
 * @tparam To Type to which cast to.
 * @tparam From Type from which cast.
 * @param from Value to cast.
 * @return Value as To.
 */
template <typename To, typename From>
[[nodiscard]] constexpr To implicit_cast(From&& from) noexcept {
  return To{from};
}

template <typename TEnum>
concept is_enum = std::is_enum_v<TEnum>;

/**
 * @brief Safely casts enum value to its underlying type.
 * @tparam TEnum Enum.
 * @param value Enum value.
 * @return Enum value with underlying enum type.
 */
template <is_enum TEnum>
constexpr std::underlying_type_t<TEnum> underlying_cast(TEnum value) noexcept {
  return std::to_underlying(value);
}

/**
 * @brief More or less safely casts from enum to enum.
 * @tparam ToEnum Enum to cast to.
 * @tparam FromEnum Enum to cast from.
 * @param from Enum to cast from.
 * @return Enum to cast from as enum to cast to.
 */
template <is_enum ToEnum, is_enum FromEnum>
[[nodiscard]] constexpr ToEnum enum_cast(FromEnum from) noexcept {
  return static_cast<ToEnum>(underlying_cast(from));
}

/**
 * @brief Unique smart pointer alias.
 * @tparam T Object type for unique_ptr.
 * @tparam Deleter Deleter for T.
 */
template <typename T, typename Deleter = std::default_delete<T>>
using un = std::unique_ptr<T, Deleter>;

}  // namespace wb::base

#endif  // !WB_BASE_MACROSES_H_
