// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Scoped handler when new operator fails to allocate memory.

#include "base/deps/g3log/g3log.h"
#include "scoped_new_handler.h"
//
#include <array>
#include <limits>
//
#include "base/deps/googletest/gtest/gtest.h"

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(ScopedNewHandlerTest, OutOfMemoryTriggersNewFailureHandler) {
  wb::base::ScopedNewHandler scoped_new_handler{
      wb::base::DefaultNewFailureHandler};

  //constexpr size_t kOomAllocSize{std::numeric_limits<unsigned>::max() >> 2U};
  //std::array<int *, 1024> memory{};

  //// TODO(dimhotepus): Fix test to actually trigger OOM.
  //for (auto &block : memory) {
  //  block = new int[kOomAllocSize];
  //  block[kOomAllocSize - 1] = 0;
  //}
}