// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// <clocale> extensions.

#ifndef WB_BASE_STD_EXT_CLOCALE_EXT_H_
#define WB_BASE_STD_EXT_CLOCALE_EXT_H_

#include <clocale>
#include <cstddef>
#include <optional>

#include "base/base_macroses.h"
#include "base/deps/g3log/g3log.h"

namespace wb::base::std_ext {
enum class ScopedProcessLocaleCategory : int;
}

/**
 * operator << for ScopedProcessLocaleCategory.
 * @param s Stream.
 * @param category Category.
 * @return Stream with dumped category.
 */
inline std::basic_ostream<char, std::char_traits<char>> &operator<<(
    std::basic_ostream<char, std::char_traits<char>> &s,
    wb::base::std_ext::ScopedProcessLocaleCategory category);

namespace wb::base::std_ext {
/**
 * Locale category.
 */
enum class ScopedProcessLocaleCategory : int {
  kAll = LC_ALL,
  kCollate = LC_COLLATE,
  kCharacterType = LC_CTYPE,
  kMessages = LC_MESSAGES,
  kMonetary = LC_MONETARY,
  kNumeric = LC_NUMERIC,
  kTime = LC_TIME
};

/**
 * Scoped process locale.
 */
class ScopedProcessLocale {
 public:
  /**
   * Creates scoped process locale.
   * @param category Locale category.
   * @param new_locale New locale.
   */
  ScopedProcessLocale(ScopedProcessLocaleCategory category,
                      const char *new_locale) noexcept
      : old_locale_{std::setlocale(underlying_cast(category), nullptr)},
        new_locale_{std::setlocale(underlying_cast(category), new_locale)},
        category_{category} {
    G3DCHECK(!!new_locale_)
        << "Locale " << new_locale << " was not set for category " << category;
  }
  ~ScopedProcessLocale() noexcept {
    if (new_locale_) {
      G3CHECK(!!std::setlocale(underlying_cast(category_), old_locale_));
      new_locale_ = nullptr;
    }
  }

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedProcessLocale);

  /**
   * Is new locale applied?
   * @return true if it is, false otherwise.
   */
  [[nodiscard]] bool IsSucceeded() const noexcept {
    return new_locale_ != nullptr;
  }

  /**
   * Gets current locale.
   * @return optional with current locale, nullopt when no current locale found.
   */
  [[nodiscard]] std::optional<const char *> GetCurrentLocale() const noexcept {
    return new_locale_
               ? std::optional{new_locale_}
               : (old_locale_ ? std::optional{old_locale_} : std::nullopt);
  }

 private:
  /**
   * Old and new locale string.
   */
  const char *old_locale_, *new_locale_;
  /**
   * Locale category.
   */
  ScopedProcessLocaleCategory category_;
  [[maybe_unused]] std::byte pad_[sizeof(char *) - sizeof(category_)];
};
}  // namespace wb::base::std_ext

/**
 * operator << for ScopedProcessLocaleCategory.
 * @param s Stream.
 * @param category Category.
 * @return Stream with dumped category.
 */
inline std::basic_ostream<char, std::char_traits<char>> &operator<<(
    std::basic_ostream<char, std::char_traits<char>> &s,
    wb::base::std_ext::ScopedProcessLocaleCategory category) {
  if (category == wb::base::std_ext::ScopedProcessLocaleCategory::kAll)
      [[unlikely]] {
    return s << "All";
  }

  if (category == wb::base::std_ext::ScopedProcessLocaleCategory::kCollate)
      [[unlikely]] {
    return s << "Collate";
  }

  if (category ==
      wb::base::std_ext::ScopedProcessLocaleCategory::kCharacterType)
      [[unlikely]] {
    return s << "Character Type";
  }

  if (category == wb::base::std_ext::ScopedProcessLocaleCategory::kMessages)
      [[unlikely]] {
    return s << "Messages";
  }

  if (category == wb::base::std_ext::ScopedProcessLocaleCategory::kMonetary)
      [[unlikely]] {
    return s << "Monetary";
  }

  if (category == wb::base::std_ext::ScopedProcessLocaleCategory::kNumeric)
      [[unlikely]] {
    return s << "Numeric";
  }

  if (category == wb::base::std_ext::ScopedProcessLocaleCategory::kTime)
      [[unlikely]] {
    return s << "Time";
  }

  return s;
}

#endif  // !WB_BASE_STD_EXT_CLOCALE_EXT_H_
