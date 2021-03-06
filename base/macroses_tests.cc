// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Provides common macroses & inline functions to use in relying apps.

#include "macroses.h"
//
#include <type_traits>
//
#include "base/deps/googletest/gtest/gtest.h"

namespace {

class NotCopyable {
  WB_NO_COPY_CTOR_AND_ASSIGNMENT(NotCopyable);
  
  NotCopyable(NotCopyable &&c) noexcept = default;
  NotCopyable& operator=(NotCopyable &&c) noexcept = default;
  ~NotCopyable() noexcept = default;
};

class NotMovable {
  NotMovable(NotMovable&) = default;
  NotMovable& operator=(NotMovable&) = default;
  ~NotMovable() noexcept = default;

  WB_NO_MOVE_CTOR_AND_ASSIGNMENT(NotMovable);
};

class NotCopyMovable {
  ~NotCopyMovable() noexcept = default;
  
  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(NotCopyMovable);
};

class Base {};

class Derived : public Base {};

enum class Enum : unsigned int { kNone = 0U };

}  // namespace

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(BaseMacrosesTest, NoCopyConstructorAndAssignment) {
  static_assert(!std::is_copy_constructible_v<NotCopyable>,
                "NotCopyable should be not copyable");
  static_assert(!std::is_copy_assignable_v<NotCopyable>,
                "NotCopyable should be not copy assignable");
}

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(BaseMacrosesTest, NoMoveConstructorAndAssignment) {
  static_assert(!std::is_move_constructible_v<NotMovable>,
                "NotMovable should be not movable");
  static_assert(!std::is_move_assignable_v<NotMovable>,
                "NotMovable should be not copy movable");
}

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(BaseMacrosesTest, NoCopyMoveConstructorAndAssignment) {
  static_assert(!std::is_copy_constructible_v<NotCopyMovable>,
                "NotCopyMovable should be not copyable");
  static_assert(!std::is_copy_assignable_v<NotCopyMovable>,
                "NotCopyMovable should be not copy assignable");

  static_assert(!std::is_move_constructible_v<NotCopyMovable>,
                "NotCopyMovable should be not movable");
  static_assert(!std::is_move_assignable_v<NotCopyMovable>,
                "NotCopyMovable should be not copy movable");
}

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(BaseMacrosesTest, ImplicitCast) {
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
  static_assert(wb::base::implicit_cast<int>(12) == 12);
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
  static_assert(wb::base::implicit_cast<unsigned>(12U) == 12U);
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
  static_assert(wb::base::implicit_cast<long long int>(12) == 12LL);
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
  static_assert(wb::base::implicit_cast<unsigned long long int>(12U) == 12ULL);

  [[maybe_unused]] Derived d;
  static_assert(wb::base::implicit_cast<Base*>(&d) == &d);
}

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(BaseMacrosesTest, UnderlyingCast) {
  static_assert(std::is_same_v<unsigned int, decltype(wb::base::underlying_cast(
                                                 Enum::kNone))>);
  static_assert(wb::base::underlying_cast(Enum::kNone) == 0U);
}

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(BaseMacrosesTest, UniquePtrAlias) {
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
  wb::base::un<int> v = std::make_unique<int>(36);

  EXPECT_EQ(*v, 36);
}