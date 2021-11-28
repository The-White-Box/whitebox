// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Counting stream buffer.

#include "counting_streambuf.h"
//
#include "base/deps/abseil/cleanup/cleanup.h"
#include "base/deps/googletest/gtest/gtest.h"

using namespace wb::base::std2;

// NOLINTNEXTLINE(cert-err58-cpp, cppcoreguidelines-avoid-non-const-global-variables, cppcoreguidelines-owning-memory)
GTEST_TEST(CountingStreamBufTest, Count) {
  countingstreambuf cout_streambuf{std::cout.rdbuf()};

  const auto old_cout_streambuf = std::cout.rdbuf(&cout_streambuf);

  const absl::Cleanup cout_streambuf_restorer = [old_cout_streambuf] {
    std::cout.rdbuf(old_cout_streambuf);
  };

  std::cout << "12345";
  EXPECT_EQ(cout_streambuf.count(), 5);
  
  std::cout << "67890";
  EXPECT_EQ(cout_streambuf.count(), 10);
  
  std::cout << "\t\n\r\a\b";
  EXPECT_EQ(cout_streambuf.count(), 15);
  
  std::cout << std::endl;
  EXPECT_EQ(cout_streambuf.count(), 16);
}
