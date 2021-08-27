// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Entry point for base tests.

#include <iostream>

#include "base/deps/g3log/scoped_g3log_initializer.h"
#include "base/deps/googletest/gtest/gtest.h"
#include "build/static_settings_config.h"

int main(int argc, char *argv[]) {
  std::cout << "Running main() from " << __FILE__ << '\n';

  // Initialize g3log logging library first as need to check contracts work.
  const wb::base::deps::g3log::ScopedG3LogInitializer scoped_g3log_initializer{
      argv[0], wb::build::settings::kPathToMainLogFile};

  testing::InitGoogleTest(&argc, argv);
  GTEST_FLAG_SET(death_test_style, "fast");
  return RUN_ALL_TESTS();
}