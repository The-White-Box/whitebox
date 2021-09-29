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
      const std::set<std::string>& locale_ids) noexcept;

  /**
   * @brief Gets localized string by message id.
   * @param message_id Message id.
   * @return Localized string.
   */
  [[nodiscard]] LookupResult<Ref<const std::string>> String(
      uint64_t message_id) const noexcept;

  /**
   * @brief Gets localized formatted string by message id.
   * @param message_id Message id.
   * @param format_args Message format args.
   * @return Localized string.
   */
  [[nodiscard]] LookupResult<std::string> Format(
      uint64_t message_id, fmt::format_args format_args) const noexcept;

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
   * |fallback_string| is used as fallback.
   * @param locale_ids Set of locale ids, order by descending preference.
   * @param fallback_string String to return if requested one not found.
   * @return Lookup.
   */
  [[nodiscard]] static LookupResult<LookupWithFallback> New(
      const std::set<std::string>& locale_ids,
      std::string fallback_string = kFallbackString) noexcept;

  /**
   * @brief Gets localized string by message id.  Returns fallback string if one
   * not found.
   * @param message_id Message id.
   * @return Localized string.
   */
  [[nodiscard]] const std::string& String(uint64_t message_id) const noexcept;

  /**
   * @brief Gets localized formatted string by message id.
   * @param message_id Message id.
   * @param format_args Message format args.
   * @return Localized string.
   */
  [[nodiscard]] std::string Format(uint64_t message_id,
                                   fmt::format_args format_args) const noexcept;

  /**
   * @brief Gets string layout.
   * @return StringLayout.
   */
  [[nodiscard]] WB_ATTRIBUTE_CONST StringLayout Layout() const noexcept;

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

/**
 * Small hasher for i18n purposes.
 */
class I18nStringViewHash {
 public:
  constexpr I18nStringViewHash() noexcept = default;

  [[nodiscard]] WB_ATTRIBUTE_CONST constexpr uint64_t operator()(
      std::string_view s, size_t index = 0) const noexcept {
    return index < s.size()
               ? (static_cast<uint64_t>(primes[index % std::size(primes)]) *
                  (index + 1) * static_cast<uint64_t>(s[index])) ^
                     (this->operator()(s, index + 1))
               : 0;
  }

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(I18nStringViewHash);

 private:
  constexpr static unsigned short primes[]{
      2,   3,   5,   7,   11,  13,  17,  19,  23,  29,  31,  37,  41,  43,  47,
      53,  59,  61,  67,  71,  73,  79,  83,  89,  97,  101, 103, 107, 109, 113,
      127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197,
      199, 211, 223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281,
      283, 293, 307, 311, 313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379,
      383, 389, 397, 401, 409, 419, 421, 431, 433, 439, 443, 449, 457, 461, 463,
      467, 479, 487, 491, 499, 503, 509, 521, 523, 541, 547, 557, 563, 569, 571,
      577, 587, 593, 599, 601, 607, 613, 617, 619, 631, 641, 643, 647, 653, 659,
      661, 673, 677, 683, 691, 701, 709, 719};
  static_assert(sizeof(primes) == 256U,
                "Should use small primes set to fit CPU cache.");
};

/**
 * Localizes |string|.
 * @param intl Localization lookup.
 * @param string String to localize.
 * @return Localized string.
 */
[[nodiscard]] WB_ATTRIBUTE_FORCEINLINE const std::string& l18n(
    const LookupWithFallback& intl, std::string_view string) noexcept {
  const uint64_t hash{I18nStringViewHash{}(string)};
  return intl.String(hash);
}

/**
 * Localizes |string| with format args |args|.
 * @param intl Localization lookup.
 * @param string String to localize.
 * @param args Format args.
 * @return Localized string.
 */
template <typename... TArgs>
[[nodiscard]] WB_ATTRIBUTE_FORCEINLINE std::string l18n_fmt(
    const LookupWithFallback& intl, std::string_view string,
    TArgs&&... args) noexcept {
  const uint64_t hash{I18nStringViewHash{}(string)};
  return intl.Format(hash, fmt::make_format_args(std::forward<TArgs>(args)...));
}
}  // namespace wb::base::intl

#endif  // !WB_BASE_INTL_LOOKUP_H_
