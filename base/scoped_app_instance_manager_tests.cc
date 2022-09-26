// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Ensures single app instance is running.

#include "scoped_app_instance_manager.h"
//
#include "base/deps/googletest/gtest/gtest.h"

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(ScopedAppInstanceManagerTest, ShouldCheckAppInstanceRunningInScope) {
  using namespace wb::base;

  {
    const ScopedAppInstanceManager scoped_app_instance_manager{"test_app"};

    EXPECT_EQ(AppInstanceStatus::kNoOtherInstances,
              scoped_app_instance_manager.GetStatus());
  }

  const ScopedAppInstanceManager scoped_app_instance_manager{"test_app"};

  EXPECT_EQ(AppInstanceStatus::kNoOtherInstances,
            scoped_app_instance_manager.GetStatus());
}

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(ScopedAppInstanceManagerTest, ShouldDetectOtherAppInstance) {
  using namespace wb::base;

  const ScopedAppInstanceManager scoped_app_instance_manager1{"test_app"};

  EXPECT_EQ(AppInstanceStatus::kNoOtherInstances,
            scoped_app_instance_manager1.GetStatus());

  const ScopedAppInstanceManager scoped_app_instance_manager2{"test_app"};

  EXPECT_EQ(AppInstanceStatus::kAlreadyRunning,
            scoped_app_instance_manager2.GetStatus());
}
