// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Mimalloc output / error handlers.

#include "mimalloc_output_handlers.h"

#include <cerrno>   // EFAULT
#include <cstdlib>  // std::strstr
#include <cstring>  // std::abort

#include "build/compiler_config.h"  // WB_COMPILER_

#ifdef WB_COMPILER_MSVC
#include <intrin.h>  // __debugbreak
#endif

#include "base/deps/g3log/g3log.h"
#include "base/deps/mimalloc/mimalloc.h"
#include "base/std2/system_error_ext.h"

#ifdef WB_MI_MALLOC
namespace {

/**
 * @brief Default mi-malloc output handler.  Function is called to output any
 * information from mimalloc, like verbose or warning messages.
 * @param msg Message to output.
 * @param arg Argument that was passed at registration to hold extra state.
 * @return void.
 */
void DefaultMiMallocOutput(const char *msg,
                           [[maybe_unused]] void *arg) noexcept {
  const bool is_warning_or_error{!!std::strstr(msg, "warning:") ||
                                 !!std::strstr(msg, "error:")};

  if (is_warning_or_error) {
    G3LOG(WARNING) << "Mi-malloc warning: " << msg;
  } else {
    G3LOG(INFO) << "Mi-malloc output: " << msg;
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
void DefaultMiMallocError(int error_no, [[maybe_unused]] void *arg) noexcept {
  const auto error_code{wb::base::std2::system_last_error_code(error_no)};
  G3PLOG_E(WARNING, error_code) << "Mi-malloc error: ";

#ifndef NDEBUG
  if (error_no == EFAULT) {
#ifdef WB_COMPILER_MSVC
    __debugbreak();
#endif
    std::abort();
  }
#endif

#if defined(NDEBUG) && defined(WB_MI_SECURE)
  if (error_no == EFAULT) {
    // Abort on serious errors in secure mode (corrupted meta-data).
    std::abort();
  }
#endif  // NDEBUG && WB_MI_SECURE

  // It as always legal to just return from the function in which case
  // allocation functions generally return NULL or ignore the condition.  The
  // default function only calls abort() when compiled in secure mode with an
  // EFAULT error.
}

void InstallMimallocOutputHandlers() noexcept {
  // Log output / errors.
  ::mi_register_output(DefaultMiMallocOutput, nullptr);
  ::mi_register_error(DefaultMiMallocError, nullptr);
}

void UninstallMimallocOutputHandlers() noexcept {
  // Log output / errors.
  ::mi_register_output(nullptr, nullptr);
  ::mi_register_error(nullptr, nullptr);
}

}  // namespace

namespace wb::base::tests_internal {

ScopedMimallocOutputHandlers::ScopedMimallocOutputHandlers() noexcept {
  InstallMimallocOutputHandlers();
}

ScopedMimallocOutputHandlers::~ScopedMimallocOutputHandlers() noexcept {
  UninstallMimallocOutputHandlers();
}

}  // namespace wb::base::tests_internal
#endif  // WB_MI_MALLOC
