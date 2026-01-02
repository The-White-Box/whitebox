// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Scoped handler when pure virtual function call is detected.

#include "scoped_process_pure_call_handler.h"
//
#include "base/deps/googletest/gtest/gtest.h"
#include "base/tests/g3log_death_utils.h"
#include "build/compiler_config.h"

namespace {

class Derived;

/**
 * @brief Base class to trigger pure call handler.
 */
class Base {
 public:
  Base(Derived *derived) noexcept : derived_{derived} {};
  virtual ~Base() noexcept;

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(Base);

  virtual void function() noexcept = 0;

  Derived *derived_;
};

/**
 * @brief Derived class to trigger pure call handler.
 */
class Derived : public Base {
 public:
  WB_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()
    // this used in member initializer by purpose.
    WB_MSVC_DISABLE_WARNING(4355)
    Derived() : Base{this} {}
  WB_MSVC_END_WARNING_OVERRIDE_SCOPE()

  void function() noexcept override {}

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(Derived);
};

Base::~Base() noexcept { derived_->function(); }

}  // namespace

using namespace wb::base::win::error_handling;

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(ScopedProcessPureCallHandlerTests,
           ShouldSetPureCallHandlerInScopeTest) {
  ASSERT_NE(::_get_purecall_handler(), DefaultPureCallHandler);

  {
    ScopedProcessPureCallHandler scoped_process_pure_call_handler{
        DefaultPureCallHandler};

    EXPECT_EQ(::_get_purecall_handler(), DefaultPureCallHandler);
  }

  EXPECT_NE(::_get_purecall_handler(), DefaultPureCallHandler);
}

#ifdef GTEST_HAS_DEATH_TEST
// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(ScopedProcessPureCallHandlerDeathTest,
           ShouldTerminateOnPureCallTest) {
  GTEST_FLAG_SET(death_test_style, "threadsafe");

  const auto triggerPureCall = []() {
    ScopedProcessPureCallHandler scoped_process_pure_call_handler{
        DefaultPureCallHandler};

    Derived derived;
  };

  const auto test_result =
      wb::base::tests_internal::MakeG3LogCheckFailureDeathTestResult(
          "Pure virtual function call occured.  Stopping the app.");

  WB_GCC_BEGIN_WARNING_OVERRIDE_SCOPE()
    WB_GCC_DISABLE_SWITCH_DEFAULT_WARNING()
    EXPECT_EXIT(triggerPureCall(), test_result.exit_predicate,
                test_result.message);
  WB_GCC_END_WARNING_OVERRIDE_SCOPE()
}
#endif  // GTEST_HAS_DEATH_TEST