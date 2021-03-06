// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Entry point for base tests.

#include <iostream>

#include "base/deps/g3log/scoped_g3log_initializer.h"
#include "base/deps/googletest/gtest/gtest.h"
#include "base/tests/mimalloc_output_handlers.h"
#include "build/static_settings_config.h"

int main(int argc, char *argv[]) {
  std::cout << "Running main() from " << __FILE__ << '\n';

  using namespace wb::base;

  // Initialize g3log logging library first as need to check contracts work.
  const deps::g3log::ScopedG3LogInitializer scoped_g3log_initializer{
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay,cppcoreguidelines-pro-bounds-pointer-arithmetic)
      argv[0], wb::build::settings::kPathToMainLogFile};

#ifdef WB_MI_MALLOC
  tests_internal::ScopedMimallocOutputHandlers scoped_mimalloc_output_handlers;
#endif

  testing::InitGoogleTest(&argc, argv);
  GTEST_FLAG_SET(death_test_style, "fast");
  return RUN_ALL_TESTS();
}