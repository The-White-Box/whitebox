// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Default new allocation failure handler.

#include "default_new_handler.h"

#include <cerrno>  // ENOMEM.
#include <chrono>
#include <cstdlib>  // std::exit.
#include <thread>

#include "base/deps/abseil/cleanup/cleanup.h"
#include "base/deps/g3log/g3log.h"
#include "base/deps/mimalloc/mimalloc.h"
#include "base/internals/scoped_new_handler_internal.h"
#include "base/scoped_new_handler.h"
#include "base/std2/string_ext.h"
#include "base/std2/thread_ext.h"
#include "build/compiler_config.h"

#ifdef WB_OS_WIN
#include "base/win/memory/memory_utils.h"
#endif

namespace {

/**
 * @brief Get current thread name.
 * @return Thread name.
 */
[[nodiscard]] std::string GetThreadName() {
  constexpr char kUnknownThreadName[]{"N/A"};

  using namespace wb::base;

  std2::native_thread_name native_thread_name;
  std::string actual_thread_name;

  if (const auto rc = std2::get_thread_name(std2::this_thread::get_handle(),
                                            native_thread_name);
      !rc)
    WB_ATTRIBUTE_LIKELY {
#ifdef WB_OS_WIN
      actual_thread_name = std2::WideToUTF8(native_thread_name);
#else
      actual_thread_name = native_thread_name;
#endif
    }

  return actual_thread_name.empty() ? kUnknownThreadName : actual_thread_name;
};

}  // namespace

namespace wb::base {

/**
 * @brief Default new memory allocation failure handler.
 * @return void.
 */
WB_BASE_API void DefaultNewFailureHandler() {
  // Mimalloc heaps are per thread, so only some threads can{not} allocate.
  static thread_local std::uint32_t actual_new_retries_count{0U};
  static thread_local bool is_recursive_new_failure{false};

  if (is_recursive_new_failure) std::exit(ENOMEM);

  const absl::Cleanup recursive_new_failure_detector_flusher{
      [&] { is_recursive_new_failure = false; }};

  is_recursive_new_failure = true;

  const uint32_t max_new_retries_count{
      internals::GetGlobalScopedNewHandlerMaxNewRetriesCount()};

  if (actual_new_retries_count < max_new_retries_count) WB_ATTRIBUTE_LIKELY {
      ++actual_new_retries_count;

      ::mi_collect(false);

#ifdef WB_OS_WIN
      const auto rc = win::memory::OptimizeHeapResourcesNow();
      G3PLOGE2_IF(WARNING, rc)
          << "Unable to optimize low-fragmentation heap (LFH) caches.";
#endif

      G3LOG(WARNING) << "Thread (" << std::this_thread::get_id() << ','
                     << GetThreadName()
                     << ") failed to allocate memory via new.  Taking "
                     << actual_new_retries_count << " retry attempt of "
                     << max_new_retries_count << '.';

      using namespace std::chrono_literals;

      // Do not hammer heap pools, give OS some time to free them.
      constexpr std::chrono::milliseconds kSleepBetweenReallocations{10ms};
      std::this_thread::sleep_for(kSleepBetweenReallocations);

      return;
    }

  G3LOG(WARNING)
      << "Thread (" << std::this_thread::get_id() << ',' << GetThreadName()
      << ") failed to allocate memory via new.  Please, ensure you have enough "
         "RAM to run the app.";
  std::exit(ENOMEM);
}

}  // namespace wb::base