// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Local memory scoper.

#include "scoped_local_memory.h"
//
#include "base/win/windows_light.h"
//
#include "base/deps/googletest/gtest/gtest.h"

using namespace wb::base::win::memory;

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-owning-memory)
GTEST_TEST(ScopedLocalMemoryTests, ShouldFreeMemoryOutOfScope) {
  constexpr std::size_t kAllocSizeInBytes{16U};

  HLOCAL memory{::LocalAlloc(LMEM_FIXED, kAllocSizeInBytes)};
  ASSERT_NE(memory, nullptr);

  {
    EXPECT_EQ(::LocalSize(memory), kAllocSizeInBytes)
        << "Memory should be allocated in the beginning of scope.";

    const ScopedLocalMemory scoped_local_memory{memory};

    EXPECT_EQ(::LocalSize(memory), kAllocSizeInBytes)
        << "Memory should be allocated in the end of scope.";
  }

  const auto filter_heap_corruption = [](unsigned int code) noexcept {
    // A heap has been corrupted?  Execute handler.  Or continue execution.
    return code == STATUS_HEAP_CORRUPTION ? EXCEPTION_EXECUTE_HANDLER
                                          : EXCEPTION_CONTINUE_EXECUTION;
  };

  const auto has_heap_corruption =
      [filter_heap_corruption](HLOCAL local) noexcept {
        __try {
          // "If the function fails, the return value is equal to a handle to the
          // local memory object."
          // See
          // https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-localfree
          return local == ::LocalFree(local);
        } __except (filter_heap_corruption(GetExceptionCode())) {
          return true;
        }
      };

  EXPECT_TRUE(has_heap_corruption(memory))
      << "Memory should be discarded out of scope.";
}
