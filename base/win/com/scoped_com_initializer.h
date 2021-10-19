// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Initializes COM in scope.

#ifndef WB_BASE_WIN_COM_SCOPED_COM_INITIALIZER_H_
#define WB_BASE_WIN_COM_SCOPED_COM_INITIALIZER_H_

#include <sal.h>

#include <cstddef>  // std::byte
#include <thread>
#include <type_traits>

#include "base/base_macroses.h"
#include "base/deps/g3log/g3log.h"
#include "base/win/system_error_ext.h"

using HRESULT = long;

_Check_return_ extern "C" WB_ATTRIBUTE_DLL_IMPORT
    HRESULT __stdcall CoInitializeEx(_In_opt_ void* pvReserved,
                                     _In_ unsigned long dwCoInit);
extern "C" WB_ATTRIBUTE_DLL_IMPORT void CoUninitialize(void);

namespace wb::base::win::com {

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
   * @return ScopedComInitializer
   */
  static std2::result<ScopedComInitializer> New(
      const ScopedComInitializerFlags flags) noexcept {
    auto init = ScopedComInitializer{flags};

    return !init.error_code()
               ? std2::result<ScopedComInitializer>{std::move(init)}
               : std2::result<ScopedComInitializer>{init.error_code()};
  }

  ScopedComInitializer(ScopedComInitializer&& i) noexcept : error_code_ {
    std::move(i.error_code_)
  }
#ifndef NDEBUG
  , thread_id_ { i.thread_id_ }
#endif
  {
    // Ensure no deinitialization occurs.
    i.error_code_ = std::error_code{EINVAL, std::generic_category()};
  }

  ScopedComInitializer& operator=(ScopedComInitializer&&) = delete;

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(ScopedComInitializer);

  /*
   * @brief Shut down COM.
   */
  ~ScopedComInitializer() noexcept {
#ifndef NDEBUG
    const std::thread::id this_thread_id{std::this_thread::get_id()};
    // COM should be freed on the same thread as it was initialized.
    G3CHECK(this_thread_id == thread_id_);
#endif

    if (!error_code()) {
      ::CoUninitialize();
    }
  }

 private:
  /**
   * @brief Error code.
   */
  std::error_code error_code_;
#ifndef NDEBUG
  /**
   * @brief COM initializing thread id.
   */
  std::thread::id thread_id_;

  [[maybe_unused]] std::array<std::byte, 4> pad_;
#endif

  /**
   * @brief Initializes COM with |coinit| flags for scope.
   * @param flags Flags.
   * @return nothing.
   */
  explicit ScopedComInitializer(const ScopedComInitializerFlags flags) noexcept
      : error_code_ {
    get_error(::CoInitializeEx(nullptr, underlying_cast(flags)))
  }
#ifndef NDEBUG
  , thread_id_ { std::this_thread::get_id() }
#endif
  { G3DCHECK(!error_code()); }

  /**
   * @brief Get COM initialization result.
   * @return Error code.
   */
  [[nodiscard]] std::error_code error_code() const noexcept {
    return error_code_;
  }
};

}  // namespace wb::base::win::com

#endif  // !WB_BASE_WIN_COM_SCOPED_COM_INITIALIZER_H_
