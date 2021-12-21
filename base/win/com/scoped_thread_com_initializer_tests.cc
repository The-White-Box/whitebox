// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Initializes COM for thread in scope.

#include "scoped_thread_com_initializer.h"
//
#include <Objbase.h>  // CoGetApartmentType
//
#include "base/deps/googletest/gtest/gtest.h"
#include "base/tests/g3log_death_utils.h"

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(ScopedThreadComInitializerTests,
           ShouldInitializeComForThreadInScopeTest) {
  APTTYPE apartment_type;
  APTTYPEQUALIFIER apartment_type_qualifier;

  HRESULT rc{::CoGetApartmentType(&apartment_type, &apartment_type_qualifier)};
  ASSERT_EQ(rc, CO_E_NOTINITIALIZED);

  using namespace wb::base::win;

  {
    auto init_result = com::ScopedThreadComInitializer::New(
        com::ScopedThreadComInitializerFlags::kApartmentThreaded);
    auto *init = wb::base::std2::get_result(init_result);
    ASSERT_NE(init, nullptr);

    rc = ::CoGetApartmentType(&apartment_type, &apartment_type_qualifier);

    EXPECT_TRUE(is_succeeded(rc));
    EXPECT_EQ(apartment_type, APTTYPE::APTTYPE_MAINSTA);
  }

  rc = ::CoGetApartmentType(&apartment_type, &apartment_type_qualifier);
  EXPECT_EQ(rc, CO_E_NOTINITIALIZED);

  {
    auto init_result = com::ScopedThreadComInitializer::New(
        com::ScopedThreadComInitializerFlags::kMultiThreaded);
    auto *init = wb::base::std2::get_result(init_result);
    ASSERT_NE(init, nullptr);

    rc = ::CoGetApartmentType(&apartment_type, &apartment_type_qualifier);

    EXPECT_TRUE(is_succeeded(rc));
    EXPECT_EQ(apartment_type, APTTYPE::APTTYPE_MTA);
  }

  rc = ::CoGetApartmentType(&apartment_type, &apartment_type_qualifier);
  EXPECT_EQ(rc, CO_E_NOTINITIALIZED);

  {
    auto init_result = com::ScopedThreadComInitializer::New(
        com::ScopedThreadComInitializerFlags::kDisableOle1Dde |
        com::ScopedThreadComInitializerFlags::kSpeedOverMemory);
    auto *init = wb::base::std2::get_result(init_result);
    ASSERT_NE(init, nullptr);

    rc = ::CoGetApartmentType(&apartment_type, &apartment_type_qualifier);

    EXPECT_TRUE(is_succeeded(rc));
    EXPECT_EQ(apartment_type, APTTYPE::APTTYPE_MTA);
  }

  rc = ::CoGetApartmentType(&apartment_type, &apartment_type_qualifier);
  EXPECT_EQ(rc, CO_E_NOTINITIALIZED);
}

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(ScopedThreadComInitializerTests,
           ShouldMoveComInitializerInScopeTest) {
  APTTYPE apartment_type;
  APTTYPEQUALIFIER apartment_type_qualifier;

  HRESULT rc{::CoGetApartmentType(&apartment_type, &apartment_type_qualifier)};
  ASSERT_EQ(rc, CO_E_NOTINITIALIZED);

  using namespace wb::base::win;

  {
    auto init_result = com::ScopedThreadComInitializer::New(
        com::ScopedThreadComInitializerFlags::kApartmentThreaded);
    auto *init = wb::base::std2::get_result(init_result);
    ASSERT_NE(init, nullptr);

    rc = ::CoGetApartmentType(&apartment_type, &apartment_type_qualifier);

    EXPECT_TRUE(is_succeeded(rc));
    EXPECT_EQ(apartment_type, APTTYPE::APTTYPE_MAINSTA);

    {
      auto moved = std::move(*init);

      rc = ::CoGetApartmentType(&apartment_type, &apartment_type_qualifier);

      EXPECT_TRUE(is_succeeded(rc));
      EXPECT_EQ(apartment_type, APTTYPE::APTTYPE_MAINSTA);
    }

    rc = ::CoGetApartmentType(&apartment_type, &apartment_type_qualifier);
    EXPECT_EQ(rc, CO_E_NOTINITIALIZED);
  }

  rc = ::CoGetApartmentType(&apartment_type, &apartment_type_qualifier);
  EXPECT_EQ(rc, CO_E_NOTINITIALIZED);
}

#ifdef GTEST_HAS_DEATH_TEST
#ifndef NDEBUG
// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(ScopedThreadComInitializerDeathTest,
           ShouldTerminateOnDoubleComInitWithUncompatibleApartmentsTest) {
  GTEST_FLAG_SET(death_test_style, "threadsafe");

  using namespace wb::base::win;

  const auto triggerComConcurrencyModelChange = []() {
    APTTYPE apartment_type;
    APTTYPEQUALIFIER apartment_type_qualifier;

    HRESULT rc{
        ::CoGetApartmentType(&apartment_type, &apartment_type_qualifier)};
    ASSERT_EQ(rc, CO_E_NOTINITIALIZED);

    auto init_result = com::ScopedThreadComInitializer::New(
        com::ScopedThreadComInitializerFlags::kMultiThreaded);
    auto *init = wb::base::std2::get_result(init_result);
    ASSERT_NE(init, nullptr);

    rc = ::CoGetApartmentType(&apartment_type, &apartment_type_qualifier);

    EXPECT_TRUE(is_succeeded(rc));
    EXPECT_EQ(apartment_type, APTTYPE::APTTYPE_MTA);

    [[maybe_unused]] const auto error_result =
        com::ScopedThreadComInitializer::New(
            com::ScopedThreadComInitializerFlags::kApartmentThreaded);
  };

  const auto test_result =
      wb::base::tests_internal::MakeG3LogCheckFailureDeathTestResult(
          "message is not checked");

  EXPECT_EXIT(triggerComConcurrencyModelChange(), test_result.exit_predicate,
              test_result.message);
}

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(ScopedThreadComInitializerDeathTest,
           ShouldTerminateOnComUninitializationOnAnotherThreadTest) {
  GTEST_FLAG_SET(death_test_style, "threadsafe");

  using namespace wb::base::win;

  APTTYPE apartment_type;
  APTTYPEQUALIFIER apartment_type_qualifier;

  HRESULT rc{::CoGetApartmentType(&apartment_type, &apartment_type_qualifier)};
  ASSERT_EQ(rc, CO_E_NOTINITIALIZED);

  auto init_result = com::ScopedThreadComInitializer::New(
      com::ScopedThreadComInitializerFlags::kMultiThreaded);
  auto *init = wb::base::std2::get_result(init_result);
  ASSERT_NE(init, nullptr);

  rc = ::CoGetApartmentType(&apartment_type, &apartment_type_qualifier);

  EXPECT_TRUE(is_succeeded(rc));
  EXPECT_EQ(apartment_type, APTTYPE::APTTYPE_MTA);

  const auto triggerComUninitializeInAnotherThread = [com =
                                                          std::move(*init)]() {
    std::thread uninitialize_thread{[c = std::move(com)]() { (void)c; }};
    uninitialize_thread.join();
  };

  const auto test_result =
      wb::base::tests_internal::MakeG3LogCheckFailureDeathTestResult(
          "COM should be freed on the same thread as it was initialized.");

  EXPECT_EXIT(triggerComUninitializeInAnotherThread(),
              test_result.exit_predicate, test_result.message);
}
#endif  // NDEBUG
#endif  // GTEST_HAS_DEATH_TEST