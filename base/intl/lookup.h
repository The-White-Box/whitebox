// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Provides an API for looking up localized message strings.
//
// Usage example:
//
// const std::string &hello_message{lookup.String(122)};
//
// or
//
// const std::string greetings{lookup.Format(123, user_name)};
//
// Based on
// https://cs.opensource.google/fuchsia/fuchsia/+/main:src/lib/intl/lookup/cpp/lookup.h

#ifndef WB_BASE_INTL_LOOKUP_H_
#define WB_BASE_INTL_LOOKUP_H_

#include <cstdint>  // uint64_t
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <variant>

#include "base/config.h"
#include "base/macroses.h"
#include "base/deps/fmt/core.h"
#include "build/compiler_config.h"

namespace wb::base::intl {

/**
 * @brief Gets string layout for used locale.
 */
enum class StringLayout {
  /**
   * @brief Left to right.
   */
  LeftToRight = 0,
  /**
   * @brief Right to left.
   */
  RightToLeft = 1
};

/**
 * @brief Lookup status.
 */
enum class Status {
  /**
   * @brief All is ok.
   */
  kOk = 0,
  /**
   * @brief Locale or string is not available.
   */
  kUnavailable = 1,
  /**
   * @brief Invalid locale is passed to New.
   */
  kArgumentError = 2
};

/**
 * @brief Lookup result.
 * @tparam T Result type.
 */
template <typename T>
using LookupResult = std::variant<T, Status>;

/**
 * @brief The API used to look up localized messages by their unique message ID.
 */
class WB_BASE_API Lookup {
 public:
  template <typename T>
  using Ref = std::reference_wrapper<T>;

  Lookup() noexcept = delete;
  WB_NO_COPY_CTOR_AND_ASSIGNMENT(Lookup);

  Lookup(Lookup&&) noexcept;
  Lookup& operator=(Lookup&&) noexcept = delete;
  ~Lookup() noexcept;

  /**
   * @brief Creates new lookup by locale ids.
   * @param locale_ids Set of locale ids, order by descending preference.
   * @return Lookup.
   */
  [[nodiscard]] static LookupResult<Lookup> New(
      const std::set<std::string_view>& locale_ids) noexcept;

  /**
   * @brief Gets localized string by message id.
   * @param message_id Message id.
   * @return Localized string.
   */
  [[nodiscard]] LookupResult<Ref<const std::string>> String(
      std::uint64_t message_id) const noexcept;

  /**
   * @brief Gets localized formatted string by message id.
   * @param message_id Message id.
   * @param format_args Message format args.
   * @return Localized string.
   */
  [[nodiscard]] LookupResult<std::string> Format(
      std::uint64_t message_id, fmt::format_args format_args) const noexcept;

  /**
   * @brief Gets string layout.
   * @return StringLayout.
   */
  [[nodiscard]] WB_ATTRIBUTE_CONST StringLayout Layout() const noexcept;

 private:
  class LookupImpl;

  WB_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()
    // Private member is not accessible to the DLL's client, including inline
    // functions.
    WB_MSVC_DISABLE_WARNING(4251)
    un<LookupImpl> impl_;
  WB_MSVC_END_WARNING_OVERRIDE_SCOPE()

  /**
   * @brief Creates lookup.
   * @param impl Lookup implementation.
   * @return nothing.
   */
  WB_CLANG_EXPLICIT Lookup(un<LookupImpl> impl) noexcept;
};

}  // namespace wb::base::intl

#endif  // !WB_BASE_INTL_LOOKUP_H_
