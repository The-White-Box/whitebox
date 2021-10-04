// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// <type_traits> extensions.

#include "type_traits_ext.h"
//
#include "base/deps/googletest/gtest/gtest.h"

using namespace wb::base::std2;

namespace {

struct A {};

struct B {
  [[nodiscard]] int fun() const & { return 0; }
};

}  // namespace

// NOLINTNEXTLINE(cert-err58-cpp, cppcoreguidelines-avoid-non-const-global-variables, cppcoreguidelines-owning-memory)
GTEST_TEST(TypeTraitsExtTests, IsFunctionPointer) {
  static_assert(!is_function_pointer<A>::value);
  static_assert(!is_function_pointer<decltype(&B::fun)>::value);
  static_assert(!is_function_pointer<void>::value);
  static_assert(!is_function_pointer<int>::value);
  static_assert(!is_function_pointer<const int>::value);
  static_assert(!is_function_pointer<const volatile int>::value);

  static_assert(!is_function_pointer<int *>::value);
  static_assert(!is_function_pointer<const int *>::value);
  static_assert(!is_function_pointer<int *const>::value);
  static_assert(!is_function_pointer<const int *const>::value);
  static_assert(!is_function_pointer<volatile int *>::value);
  static_assert(!is_function_pointer<const volatile int *>::value);
  static_assert(!is_function_pointer<volatile int *const>::value);
  static_assert(!is_function_pointer<const volatile int *const>::value);

  static_assert(!is_function_pointer<int()>::value);
  static_assert(!is_function_pointer<int() &>::value);
  static_assert(!is_function_pointer<int() noexcept>::value);
  static_assert(!is_function_pointer<int() const>::value);
  static_assert(!is_function_pointer<int() const noexcept>::value);

  static_assert(is_function_pointer<void (*)()>::value);
  static_assert(is_function_pointer<void (*)() noexcept>::value);
  static_assert(is_function_pointer<int (*)(int)>::value);
  static_assert(is_function_pointer<int (*)(int) noexcept>::value);
}

// NOLINTNEXTLINE(cert-err58-cpp, cppcoreguidelines-avoid-non-const-global-variables, cppcoreguidelines-owning-memory)
GTEST_TEST(TypeTraitsExtTests, IsFunctionPointerV) {
  static_assert(!is_function_pointer_v<A>);
  static_assert(!is_function_pointer_v<decltype(&B::fun)>);
  static_assert(!is_function_pointer_v<void>);
  static_assert(!is_function_pointer_v<int>);
  static_assert(!is_function_pointer_v<const int>);
  static_assert(!is_function_pointer_v<const volatile int>);

  static_assert(!is_function_pointer_v<int *>);
  static_assert(!is_function_pointer_v<const int *>);
  static_assert(!is_function_pointer_v<int *const>);
  static_assert(!is_function_pointer_v<const int *const>);
  static_assert(!is_function_pointer_v<volatile int *>);
  static_assert(!is_function_pointer_v<const volatile int *>);
  static_assert(!is_function_pointer_v<volatile int *const>);
  static_assert(!is_function_pointer_v<const volatile int *const>);

  static_assert(!is_function_pointer_v<int()>);
  static_assert(!is_function_pointer_v<int() &>);
  static_assert(!is_function_pointer_v<int() noexcept>);
  static_assert(!is_function_pointer_v<int() const>);
  static_assert(!is_function_pointer_v<int() const noexcept>);

  static_assert(is_function_pointer_v<void (*)()>);
  static_assert(is_function_pointer_v<void (*)() noexcept>);
  static_assert(is_function_pointer_v<int (*)(int)>);
  static_assert(is_function_pointer_v<int (*)(int) noexcept>);
  static_assert(is_function_pointer_v<int (*)(int) noexcept>);
}

// NOLINTNEXTLINE(cert-err58-cpp, cppcoreguidelines-avoid-non-const-global-variables, cppcoreguidelines-owning-memory)
GTEST_TEST(TypeTraitsExtTests, IsChar) {
  static_assert(!is_char<A>::value);
  static_assert(!is_char<decltype(&B::fun)>::value);
  static_assert(!is_char<void>::value);
  static_assert(!is_char<short>::value);
  static_assert(!is_char<int>::value);
  static_assert(!is_char<unsigned>::value);
  static_assert(!is_char<long>::value);
  static_assert(!is_char<const int>::value);
  static_assert(!is_char<const volatile int>::value);

  static_assert(!is_char<int *>::value);
  static_assert(!is_char<const int *>::value);
  static_assert(!is_char<int *const>::value);
  static_assert(!is_char<const int *const>::value);
  static_assert(!is_char<volatile int *>::value);
  static_assert(!is_char<const volatile int *>::value);
  static_assert(!is_char<volatile int *const>::value);
  static_assert(!is_char<const volatile int *const>::value);

  static_assert(!is_char<int()>::value);
  static_assert(!is_char<int() &>::value);
  static_assert(!is_char<int() noexcept>::value);
  static_assert(!is_char<int() const>::value);
  static_assert(!is_char<int() const noexcept>::value);

  static_assert(is_char<char>::value);
  static_assert(is_char<const char>::value);
  static_assert(is_char<volatile char>::value);
  static_assert(is_char<const volatile char>::value);
  static_assert(is_char<wchar_t>::value);
  static_assert(is_char<const wchar_t>::value);
  static_assert(is_char<volatile wchar_t>::value);
  static_assert(is_char<const volatile wchar_t>::value);
  static_assert(is_char<char8_t>::value);
  static_assert(is_char<const char8_t>::value);
  static_assert(is_char<volatile char8_t>::value);
  static_assert(is_char<const volatile char8_t>::value);
  static_assert(is_char<char16_t>::value);
  static_assert(is_char<const char16_t>::value);
  static_assert(is_char<volatile char16_t>::value);
  static_assert(is_char<const volatile char16_t>::value);
  static_assert(is_char<char32_t>::value);
  static_assert(is_char<const char32_t>::value);
  static_assert(is_char<volatile char32_t>::value);
  static_assert(is_char<const volatile char32_t>::value);
}

// NOLINTNEXTLINE(cert-err58-cpp, cppcoreguidelines-avoid-non-const-global-variables, cppcoreguidelines-owning-memory)
GTEST_TEST(TypeTraitsExtTests, IsCharV) {
  static_assert(!is_char_v<A>);
  static_assert(!is_char_v<decltype(&B::fun)>);
  static_assert(!is_char_v<void>);
  static_assert(!is_char_v<short>);
  static_assert(!is_char_v<int>);
  static_assert(!is_char_v<unsigned>);
  static_assert(!is_char_v<long>);
  static_assert(!is_char_v<const int>);
  static_assert(!is_char_v<const volatile int>);

  static_assert(!is_char_v<int *>);
  static_assert(!is_char_v<const int *>);
  static_assert(!is_char_v<int *const>);
  static_assert(!is_char_v<const int *const>);
  static_assert(!is_char_v<volatile int *>);
  static_assert(!is_char_v<const volatile int *>);
  static_assert(!is_char_v<volatile int *const>);
  static_assert(!is_char_v<const volatile int *const>);

  static_assert(!is_char_v<int()>);
  static_assert(!is_char_v<int() &>);
  static_assert(!is_char_v<int() noexcept>);
  static_assert(!is_char_v<int() const>);
  static_assert(!is_char_v<int() const noexcept>);

  static_assert(is_char_v<char>);
  static_assert(is_char_v<const char>);
  static_assert(is_char_v<volatile char>);
  static_assert(is_char_v<const volatile char>);
  static_assert(is_char_v<wchar_t>);
  static_assert(is_char_v<const wchar_t>);
  static_assert(is_char_v<volatile wchar_t>);
  static_assert(is_char_v<const volatile wchar_t>);
  static_assert(is_char_v<char8_t>);
  static_assert(is_char_v<const char8_t>);
  static_assert(is_char_v<volatile char8_t>);
  static_assert(is_char_v<const volatile char8_t>);
  static_assert(is_char_v<char16_t>);
  static_assert(is_char_v<const char16_t>);
  static_assert(is_char_v<volatile char16_t>);
  static_assert(is_char_v<const volatile char16_t>);
  static_assert(is_char_v<char32_t>);
  static_assert(is_char_v<const char32_t>);
  static_assert(is_char_v<volatile char32_t>);
  static_assert(is_char_v<const volatile char32_t>);
}
