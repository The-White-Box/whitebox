// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Provides an API for looking up localized message strings.
//
// Based on
// https://cs.opensource.google/fuchsia/fuchsia/+/main:src/lib/intl/lookup/cpp/lookup.h

#ifndef WB_BASE_INTL_LOOKUP_H_
#define WB_BASE_INTL_LOOKUP_H_

#include <cstdint>
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <variant>

#include "base/base_api.h"
#include "base/base_macroses.h"
#include "base/deps/fmt/core.h"
#include "base/intl/message_ids.h"
#include "build/compiler_config.h"

namespace wb::base::intl {
/**
 * @brief The API used to look up localized messages by their unique message ID.
 */
class WB_BASE_API Lookup {
 public:
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
    kArgumentError = 2,
    /**
     * @brief Internal error occured.
     */
    kInternal = 3
  };

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

  template <typename T>
  using LookupResult = std::variant<T, Lookup::Status>;

  template <typename T>
  using ref = std::reference_wrapper<T>;

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
      const std::set<std::string>& locale_ids) noexcept;

  /**
   * @brief Gets localized string by message id.
   * @param message_id Message id.
   * @return Localized string.
   */
  [[nodiscard]] LookupResult<ref<const std::string>> String(
      uint64_t message_id) noexcept;

  /**
   * @brief Gets localized formatted string by message id.
   * @param message_id Message id.
   * @param format_args Message format args.
   * @return Localized string.
   */
  [[nodiscard]] LookupResult<std::string> StringFormat(
      uint64_t message_id, fmt::format_args format_args) noexcept;

  /**
   * @brief Gets string layout.
   * @return StringLayout.
   */
  [[nodiscard]] StringLayout Layout() const noexcept;

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
  Lookup(un<LookupImpl> impl) noexcept;
};

/**
 * @brief Fallback string.
 */
constexpr char kFallbackString[]{"N/A"};

/**
 * @brief The API used to look up localized messages by their unique message ID.
 */
class WB_BASE_API LookupWithFallback {
 public:
  LookupWithFallback() noexcept = delete;
  WB_NO_COPY_CTOR_AND_ASSIGNMENT(LookupWithFallback);

  LookupWithFallback(LookupWithFallback&&) noexcept;
  Lookup& operator=(LookupWithFallback&&) noexcept = delete;

  /**
   * @brief Creates new lookup by locale ids.  If no string found
   * @param locale_ids Set of locale ids, order by descending preference.
   * @param fallback_string String to return if requested one not found.
   * @return Lookup.
   */
  [[nodiscard]] static Lookup::LookupResult<LookupWithFallback> New(
      const std::set<std::string>& locale_ids,
      std::string fallback_string = kFallbackString) noexcept;

  /**
   * @brief Gets localized string by message id.  Returns fallback string if one
   * not found.
   * @param message_id Message id.
   * @return Localized string.
   */
  [[nodiscard]] const std::string& String(uint64_t message_id) noexcept;

  /**
   * @brief Gets localized formatted string by message id.
   * @param message_id Message id.
   * @param format_args Message format args.
   * @return Localized string.
   */
  [[nodiscard]] std::string StringFormat(uint64_t message_id,
                                         fmt::format_args format_args) noexcept;

  /**
   * @brief Gets string layout.
   * @return StringLayout.
   */
  [[nodiscard]] Lookup::StringLayout Layout() const noexcept;

 private:
  Lookup lookup_;
  WB_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()
    // Private member is not accessible to the DLL's client, including inline
    // functions.
    WB_MSVC_DISABLE_WARNING(4251)
    std::string fallback_string_;
  WB_MSVC_END_WARNING_OVERRIDE_SCOPE()

  /**
   * @brief Creates lookup with fallback.
   * @param lookup Lookup.
   * @param fallback_string Fallback string.
   * @return nothing.
   */
  LookupWithFallback(Lookup lookup, std::string fallback_string) noexcept;
};
}  // namespace wb::base::intl

#endif  // !WB_BASE_INTL_LOOKUP_H_
