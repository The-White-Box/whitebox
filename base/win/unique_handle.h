// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// HANDLE RAII wrapper.

#ifndef WB_BASE_WIN_UNIQUE_HANDLE_H_
#define WB_BASE_WIN_UNIQUE_HANDLE_H_

#include <cstdint>  // std::uintptr_t
#include <memory>   // std::unique_ptr

#include "base/deps/g3log/g3log.h"
#include "base/std2/system_error_ext.h"
#include "build/build_config.h"
#include "build/compiler_config.h"

using HANDLE = void *;
extern "C" WB_ATTRIBUTE_DLL_IMPORT int __stdcall CloseHandle(
    _In_ _Post_ptr_invalid_ HANDLE hObject);

namespace wb::base::win {

/**
 * @brief Pseudo handle.
 */
struct alignas(void *) HANDLE_ {
  size_t unused;
};

/**
 * @brief Handle descriptor.  Do not use native void* here as it means smb can
 * accidentally delete void* ptr via module deleter which is not we want.
 */
using handle_descriptor = HANDLE_;

/**
 * @brief Invalid native handle.
 */
inline const HANDLE kInvalidNativeHandle{
    reinterpret_cast<HANDLE>(static_cast<std::uintptr_t>(-1))};

}  // namespace wb::base::win

namespace std {

/**
 * @brief Deleter to close HANDLE on end of scope.
 */
template <>
struct default_delete<wb::base::win::handle_descriptor> {
  /**
   * @brief Closes handle.
   * @param handle Handle to close.
   * @return void.
   */
  void operator()(
      _In_ wb::base::win::handle_descriptor *handle) const noexcept {
    if (handle && handle != wb::base::win::kInvalidNativeHandle) {
      const std::error_code rc{::CloseHandle(handle) != 0
                                   ? wb::base::std2::ok_code
                                   : wb::base::std2::system_last_error_code()};
      G3PCHECK_E(!rc, rc) << "Close handle failed.";
    }
  }
};

}  // namespace std

namespace wb::base::win {

/**
 * @brief Smart pointer with std::unique_ptr semantic for HANDLE lifecycle
 * control.
 */
class unique_handle : private std::unique_ptr<handle_descriptor> {
  using unique_handle_base =
      std::unique_ptr<handle_descriptor,
                      std::default_delete<handle_descriptor>>;

 public:
  /**
   * @brief Get existing handle.
   */
  [[nodiscard]] WB_CONSTEXPR_CXX23 void *get() const noexcept {
    return unique_handle_base::get();
  }

  /**
   * @brief Reset handle.
   */
  WB_CONSTEXPR_CXX23 void reset(void *handle = nullptr) noexcept {
    unique_handle_base::reset(static_cast<handle_descriptor *>(handle));
  }

  /**
   * @brief Create handle from existing descriptor.
   * @param handle Handle to own.
   * @return nothing.
   */
  explicit unique_handle(_In_opt_ void *handle) noexcept
      : unique_handle_base{static_cast<handle_descriptor *>(handle)} {}

  unique_handle(unique_handle &&) noexcept = default;
  unique_handle &operator=(unique_handle &&) noexcept = default;

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(unique_handle);

  /**
   * @brief Check handle is a valid one.
   * @return true if valid, false otherwise.
   */
  [[nodiscard]] explicit operator bool() const noexcept {
    return get() != nullptr && get() != kInvalidNativeHandle;
  }
};

}  // namespace wb::base::win

#endif  // !WB_BASE_WIN_UNIQUE_HANDLE_H_
