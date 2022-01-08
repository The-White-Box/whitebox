// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Scoped handler for situations when new operator fails to allocate memory.

#ifndef WB_BASE_SCOPED_NEW_HANDLER_H_
#define WB_BASE_SCOPED_NEW_HANDLER_H_

#include <array>
#include <cstddef>  // byte.
#include <cstdint>  // uint16_t.
#include <new>

#include "base/config.h"
#include "base/internals/scoped_new_handler_internal.h"
#include "base/macroses.h"

namespace wb::base {

/**
 * @brief Changes handler when new operator fails to allocate memory and reverts
 * back when out of scope.  Note, if a user-defined operator new is provided,
 * the new handler functions are not automatically called on failure!
 */
class ScopedNewHandler {
 public:
  /**
   * @brief Set handler when new operator fails to allocate memory.
   * @param new_handler Handler.
   * @param max_new_retries_count Max retries count for new operator to
   * reallocate memory.
   * @return nothing.
   */
  ScopedNewHandler(std::new_handler new_handler,
                   std::uint32_t max_new_retries_count) noexcept
      : previous_new_handler_{new_handler != dummy_new_handler
                                  ? std::set_new_handler(new_handler)
                                  : dummy_new_handler},
        max_new_retries_count_{max_new_retries_count} {}

  /**
   * @brief Set dummy handler for new operator when it fails to allocate memory.
   * @return nothing.
   */
  ScopedNewHandler() noexcept : ScopedNewHandler{dummy_new_handler, 0U} {}

  ScopedNewHandler(ScopedNewHandler&& h) noexcept
      : previous_new_handler_{h.previous_new_handler_},
        max_new_retries_count_{h.max_new_retries_count_} {
    h.previous_new_handler_ = dummy_new_handler;
    h.max_new_retries_count_ = 0U;
  }
  ScopedNewHandler& operator=(ScopedNewHandler&& h) noexcept {
    std::swap(previous_new_handler_, h.previous_new_handler_);
    std::swap(max_new_retries_count_, h.max_new_retries_count_);
    return *this;
  }

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(ScopedNewHandler);

  /**
   * @brief Restore previous new handler.
   */
  ~ScopedNewHandler() noexcept {
    if (previous_new_handler_ != dummy_new_handler) {
      std::set_new_handler(previous_new_handler_);
    }
  }

  /**
   * @brief Get max retries count for the new operator to reallocate memory.
   * @return Max retries count for new.
   */
  [[nodiscard]] std::uint32_t max_new_retries_count() const noexcept {
    return max_new_retries_count_;
  }

 private:
  /**
   * @brief Dummy new failure handler.  Used to distinguish inactive / moved
   * from instances.
   */
  static inline const std::new_handler dummy_new_handler{[]() {}};

  /**
   * @brief Previous new handler.
   */
  std::new_handler previous_new_handler_;
  /**
   * @brief Max retries to allocate memory.
   */
  std::uint32_t max_new_retries_count_;

  WB_ATTRIBUTE_UNUSED_FIELD std::array<std::byte, 4> pad_;
};

/**
 * @brief Installs global new failure handler.
 * @return Previous global new failure handler.  Dummy one when no global new
 * handler installed.
 */
WB_BASE_API ScopedNewHandler
InstallGlobalScopedNewHandler(ScopedNewHandler&& handler) noexcept;

}  // namespace wb::base

#endif  // !WB_BASE_SCOPED_NEW_HANDLER_H_
