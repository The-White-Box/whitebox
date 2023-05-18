// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Provides an API for looking up localized message strings.
//
// Usage example:
//
// const std::string &hello_message{intl::l18n(intl, "Hello, international
// world!")};
//
// or
//
// const std::string greetings{intl::l18n_fmt(intl, "Hi, {0}!", user_name)};
//
// Based on
// https://cs.opensource.google/fuchsia/fuchsia/+/main:src/lib/intl/lookup/cpp/lookup.h

#ifndef WB_BASE_INTL_L18N_H_
#define WB_BASE_INTL_L18N_H_

#include <cstdint>  // uint64_t
#include <string>
#include <string_view>

#include "base/config.h"
#include "base/macroses.h"
#include "base/deps/fmt/core.h"
#include "base/intl/lookup_with_fallback.h"
#include "build/compiler_config.h"

namespace wb::base::intl {

/**
 * Small hasher for i18n purposes.
 */
class I18nStringViewHash {
 public:
  /**
   * Creates string_view hash for i18n.
   */
  constexpr I18nStringViewHash() noexcept = default;

  /**
   * Computes string_view hash.
   * @param s string_view
   * @param index start position of |s| to compute hash from till |s.size()|.
   * @return Hash.
   */
  [[nodiscard]] WB_ATTRIBUTE_CONST constexpr std::uint64_t operator()(
      std::string_view s, size_t index = 0) const noexcept {
    return index < s.size()
               ? (static_cast<std::uint64_t>(
                      primes[index % std::size(primes)]) *
                  (index + 1) * static_cast<std::uint64_t>(s[index])) ^
                     (this->operator()(s, index + 1))
               : 0;
  }

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(I18nStringViewHash);

 private:
  /**
   * @brief Primes to use in hash computation.
   */
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
 * @param lookup Localization lookup.
 * @param string String to localize.
 * @return Localized string.
 */
[[nodiscard]] inline const std::string& l18n(
    const LookupWithFallback& lookup, std::string_view&& string) noexcept {
  const std::uint64_t hash{I18nStringViewHash{}(string)};
  return lookup.String(hash);
}

/**
 * Localizes |string| with format args |args|.
 * @param lookup Localization lookup.
 * @param string String to localize.
 * @param args Format args.
 * @return Localized string.
 */
template <typename... TArgs>
[[nodiscard]] inline std::string l18n_fmt(const LookupWithFallback& lookup,
                                          std::string_view&& string,
                                          TArgs&&... args) noexcept {
  const std::uint64_t hash{I18nStringViewHash{}(string)};
  // No forwarding, as fmt::make_format_args expect references :(.
  return lookup.Format(hash,
                       fmt::make_format_args(args...));
}

}  // namespace wb::base::intl

#endif  // !WB_BASE_INTL_L18N_H_
