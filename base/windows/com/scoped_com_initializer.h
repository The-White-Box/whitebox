// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Initializes COM in scope.

#ifndef WB_BASE_WINDOWS_COM_SCOPED_COM_INITIALIZER_H_
#define WB_BASE_WINDOWS_COM_SCOPED_COM_INITIALIZER_H_

#include <sal.h>

#include <cstddef>  // std::byte
#include <thread>
#include <type_traits>

#include "base/base_macroses.h"
#include "base/deps/g3log/g3log.h"
#include "base/windows/system_error_ext.h"

using HRESULT = long;

_Check_return_ extern "C" WB_ATTRIBUTE_DLL_IMPORT
    HRESULT __stdcall CoInitializeEx(_In_opt_ void* pvReserved,
                                     _In_ unsigned long dwCoInit);
extern "C" WB_ATTRIBUTE_DLL_IMPORT void CoUninitialize(void);

namespace wb::base::windows::com {
/**
 * @brief COM initilizer flags.
 */
enum class ScopedComInitializerFlags : unsigned long {
  /**
   * @brief Initializes the thread for apartment-threaded object concurrency.
   */
  kApartmentThreaded = 0x2UL,
  /**
   * @brief Initializes the thread for multithreaded object concurrency.
   */
  kMultiThreaded = 0x0UL,
  /**
   * @brief Disables DDE for OLE1 support.
   */
  kDisableOle1Dde = 0x4UL,
  /**
   * @brief Increase memory usage in an attempt to increase performance.
   */
  kSpeedOverMemory = 0x8UL
};

/**
 * @brief operator|.
 * @param left Left.
 * @param right Right.
 * @return Left | Right.
 */
[[nodiscard]] constexpr ScopedComInitializerFlags operator|(
    ScopedComInitializerFlags left, ScopedComInitializerFlags right) noexcept {
  return static_cast<ScopedComInitializerFlags>(underlying_cast(left) |
                                                underlying_cast(right));
}

/**
 * @brief Initializes Component Object Model at scope level.
 */
class ScopedComInitializer {
 public:
  /**
   * @brief Initializes COM with |coinit| flags for scope.
   * @param flags Flags.
   * @return nothing.
   */
  explicit ScopedComInitializer(const ScopedComInitializerFlags flags) noexcept
      : error_code_ {
    GetErrorCode(::CoInitializeEx(nullptr, underlying_cast(flags)))
  }
#ifndef NDEBUG
  , thread_id_ { std::this_thread::get_id() }
#endif
  { G3DCHECK(!error_code()); }

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedComInitializer);

  /*
   * @brief Shut down COM.
   */
  ~ScopedComInitializer() noexcept {
#ifndef NDEBUG
    const std::thread::id this_thread_id{std::this_thread::get_id()};
    // COM should be freed on the same thread as it was initialized.
    G3CHECK(this_thread_id == thread_id_);
#endif

    if (!error_code()) ::CoUninitialize();
  }

  /**
   * @brief Get COM initialization result.
   * @return Error code.
   */
  [[nodiscard]] std::error_code error_code() const noexcept {
    return error_code_;
  }

 private:
  /**
   * @brief Error code.
   */
  const std::error_code error_code_;
#ifndef NDEBUG
  /**
   * @brief COM initializing thread id.
   */
  const std::thread::id thread_id_;
  [[maybe_unused]] std::byte pad_[4];
#endif
};
}  // namespace wb::base::windows::com

#endif  // !WB_BASE_WINDOWS_COM_SCOPED_COM_INITIALIZER_H_
