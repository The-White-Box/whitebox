// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Common macroses & utilities.  Common means really common!

#ifndef WHITEBOX_BASE_INCLUDE_BASE_MACROSES_H_
#define WHITEBOX_BASE_INCLUDE_BASE_MACROSES_H_

#include <memory>

/**
 * Deletes copy ctor and assignment operator for type.
 */
#define WHITEBOX_NO_COPY_CTOR_AND_ASSIGNMENT(typeName) \
  typeName(typeName&) = delete;                        \
  typeName& operator=(typeName&) = delete

/**
 * Deletes move ctor and assignment operator for type.
 */
#define WHITEBOX_NO_MOVE_CTOR_AND_ASSIGNMENT(typeName) \
  typeName(typeName&&) = delete;                       \
  typeName& operator=(typeName&&) = delete

/**
 * Deletes copy / move ctors and assignment operators for type.
 */
#define WHITEBOX_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(typeName) \
  WHITEBOX_NO_COPY_CTOR_AND_ASSIGNMENT(typeName);           \
  WHITEBOX_NO_MOVE_CTOR_AND_ASSIGNMENT(typeName)

namespace whitebox::base {
/**
 * @brief Implicit, means checked at compile time cast.
 * @tparam To Type to which cast to.
 * @tparam From Type from which cast.
 * @param from Value to cast.
 * @return Value as To.
 */
template <typename To, typename From>
constexpr To implicit_cast(From&& from) noexcept {
  return To{from};
}

/**
 * @brief Unique ptr alias.
 * @tparam T Type for unique_ptr.
 */
template <typename T>
using U = std::unique_ptr<T, std::default_delete<T>>;
}  // namespace whitebox::base

#endif  // !WHITEBOX_BASE_INCLUDE_BASE_MACROSES_H_
