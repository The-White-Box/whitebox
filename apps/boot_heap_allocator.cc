// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Heap allocator boot.

#include "boot_heap_allocator.h"

#include <cerrno>   // EFAULT
#include <cstdlib>  // std::strstr
#include <cstring>  // std::abort

#include "base/deps/g3log/g3log.h"
#include "build/compiler_config.h"  // WB_COMPILER_

#ifdef WB_MI_MALLOC
#include "base/deps/mimalloc/mimalloc.h"
#include "base/std2/system_error_ext.h"

#ifdef WB_COMPILER_MSVC
#include <intrin.h>  // __debugbreak
#endif
#endif  //  WB_MI_MALLOC

#ifdef WB_OS_WIN
#include "base/win/memory/memory_utils.h"
#endif

namespace wb::apps {

#ifdef WB_MI_MALLOC
/**
 * @brief Default mi-malloc output handler.  Function is called to output any
 * information from mimalloc, like verbose or warning messages.
 * @param msg Message to output.
 * @param arg Argument that was passed at registration to hold extra state.
 * @return void.
 */
void DefaultMiMallocOutput(const char* msg,
                           [[maybe_unused]] void* arg) noexcept {
  if (msg && msg[0] && !msg[1] && msg[0] != '\n') WB_ATTRIBUTE_LIKELY {
      const bool is_warning_or_error{std::strstr(msg, "warning:") != nullptr ||
                                     std::strstr(msg, "error:") != nullptr};

      if (is_warning_or_error) {
        G3LOG(WARNING) << "Mi-malloc warning: " << msg;
      } else {
        G3LOG(INFO) << "Mi-malloc output: " << msg;
      }
    }
}

/**
 * @brief Default mi-malloc error handler.
 * @param error_no The possible error codes are:
 * * EAGAIN: Double free was detected (only in debug and secure mode).
 * * EFAULT: Corrupted free list or meta-data was detected (only in debug and
 * secure mode).
 * * ENOMEM: Not enough memory available to satisfy the request.
 * * EOVERFLOW: Too large a request, for example in mi_calloc(), the count and
 * size parameters are too large.
 * * EINVAL: Trying to free or re-allocate an invalid pointer.
 * @param arg Extra argument that will be passed on to the error function.
 * @return void.
 */
void DefaultMiMallocError(int error_no, [[maybe_unused]] void* arg) noexcept {
  const auto error_code{base::std2::system_last_error_code(error_no)};
  G3PLOG_E(WARNING, error_code) << "Mi-malloc error: ";

#ifndef NDEBUG
  if (error_no == EFAULT) {
#ifdef WB_COMPILER_MSVC
    __debugbreak();
#endif
    std::abort();
  }
#else
#ifdef WB_MI_SECURE
  if (error_no == EFAULT) {
    // Abort on serious errors in secure mode (corrupted meta-data).
    std::abort();
  }
#endif
#endif  // NDEBUG

  // It as always legal to just return from the function in which case
  // allocation functions generally return NULL or ignore the condition.  The
  // default function only calls abort() when compiled in secure mode with an
  // EFAULT error.
}
#endif  // WB_MI_MALLOC

void BootHeapAllocator() noexcept {
#ifdef WB_OS_WIN
  {
    // Terminate the app if system detected heap corruption.
    const auto error_code =
        base::win::memory::EnableTerminationOnHeapCorruption();
    G3PLOGE2_IF(WARNING, error_code)
        << "Can't enable 'Terminate on Heap corruption' os feature, continue "
           "without it.";
  }
#endif  // WB_OS_WIN

#ifdef WB_MI_MALLOC
  G3DLOG(INFO) << "Using mi-malloc memory allocator v." << mi_version() << ".";

  // Log output / errors.
  mi_register_output(DefaultMiMallocOutput, nullptr);
  mi_register_error(DefaultMiMallocError, nullptr);
#endif  // WB_MI_MALLOC
}

}  // namespace wb::apps
