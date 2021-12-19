// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Sets process locale till ScopedProcessLocale object lifetime end.

#ifndef WB_BASE_INTL_SCOPED_PROCESS_LOCALE_H_
#define WB_BASE_INTL_SCOPED_PROCESS_LOCALE_H_

#include <clocale>
#include <cstddef>
#include <optional>
#include <string>

#include "base/deps/g3log/g3log.h"
#include "base/macroses.h"

namespace wb::base::intl {

/**
 * @brief Forward declare ScopedProcessLocaleCategory.
 */
enum class ScopedProcessLocaleCategory : decltype(LC_ALL);

namespace locales {

#ifdef WB_OS_WIN
/**
 * @brief Starting in Windows 10 build 17134 (April 2018 Update), the Universal
 * C Runtime supports using a UTF-8 code page.  This means that char strings
 * passed to C runtime functions will expect strings in the UTF-8 encoding.  For
 * example, setlocale(LC_ALL, ".UTF8") will use the current default Windows ANSI
 * code page (ACP) for the locale and UTF-8 for the code page.
 */
constexpr char kUtf8Locale[]{".UTF8"};
#elif defined(WB_OS_POSIX)
/*
 * @brief Modern Linux distributions use UTF-8 as default locale.
 */
constexpr char kUtf8Locale[]{""};
#else
#error "Please define UTF-8 locale for your os."
#endif

/**
 * @brief Fallback locale.
 */
constexpr char kFallbackLocale[]{"en_US.UTF8"};

}  // namespace locales

}  // namespace wb::base::intl

#ifdef LC_MESSAGES
/**
 * @brief Locale categories contain message one.
 */
#define WB_LOCALE_HAS_MESSAGES_CATEGORY 1
#endif

/**
 * operator << for ScopedProcessLocaleCategory.
 * @param s Stream.
 * @param category Category.
 * @return Stream with dumped category.
 */
inline std::basic_ostream<char, std::char_traits<char>> &operator<<(
    std::basic_ostream<char, std::char_traits<char>> &s,
    wb::base::intl::ScopedProcessLocaleCategory category);

namespace wb::base::intl {

/**
 * Locale category.
 */
enum class ScopedProcessLocaleCategory : decltype(LC_ALL) {
  /**
   * @brief All locale-specific behavior (all categories).
   */
  kAll = LC_ALL,
  /**
   * @brief Behavior of strcoll and strxfrm functions.
   */
  kCollate = LC_COLLATE,
  /**
   * @brief Behavior of character-handling functions (except isdigit, isxdigit,
   * mbstowcs, and mbtowc, which are unaffected).
   */
  kCharacterType = LC_CTYPE,

#ifdef WB_LOCALE_HAS_MESSAGES_CATEGORY
  /**
   * @brief Behavior of message display locales of system functions, like
   * strerror.
   */
  kMessages = LC_MESSAGES,
#endif

  /**
   * @brief Monetary formatting information returned by the localeconv function.
   */
  kMonetary = LC_MONETARY,
  /**
   * @brief Decimal-point character for formatted output routines (for example,
   * printf), data conversion routines, and nonmonetary formatting information
   * returned by localeconv function.
   */
  kNumeric = LC_NUMERIC,
  /**
   * @brief Behavior of strftime function.
   */
  kTime = LC_TIME
};

#ifndef NDEBUG
/**
 * Gets user friendly locale name.
 * @param original_name Original locale name.
 * @return User friendly locale name.
 */
[[nodiscard]] WB_ATTRIBUTE_PURE static std::string_view
GetUserFriendlyLocaleName(const char *original_name) noexcept {
  std::string_view candidate{original_name};
  return candidate.empty() ? "<empty>" : candidate;
}
#endif  // !NDEBUG

/**
 * Sets process locale category in scope, restores out of scope.
 */
class ScopedProcessLocale {
 public:
  /**
   * Creates scoped process locale.
   * @param category Locale category.
   * @param new_locale New locale.  On Linux distros locale name is typically of
   * the form "language[_territory][.codeset][\@modifier]", where language is an
   * ISO 639 language code, territory is an ISO 3166 country code, and codeset
   * is a character set or encoding identifier like ISO-8859-1 or UTF-8.  For a
   * list of all supported locales, try "locale -a".
   */
  ScopedProcessLocale(ScopedProcessLocaleCategory category,
                      const char *new_locale) noexcept
      : old_locale_{SetLocale(category, nullptr)},
        new_locale_{SetLocale(category, new_locale)},
        category_{category} {
#ifndef NDEBUG
    G3DCHECK(!new_locale_.empty())
        << "Locale " << GetUserFriendlyLocaleName(new_locale)
        << " was not set for category " << category;
#endif
  }
  ~ScopedProcessLocale() noexcept {
    // For example, the sequence of calls
    // Set all categories and return "en-US"
    //   setlocale(LC_ALL, "en-US");
    // Set only the LC_MONETARY category and return "fr-FR"
    //   setlocale(LC_MONETARY, "fr-FR");
    //   printf("%s\n", setlocale(LC_ALL, NULL));
    // returns
    //   LC_COLLATE=en-US;LC_CTYPE=en-US;LC_MONETARY=fr-FR;LC_NUMERIC=en-US;LC_TIME=en-US
    //
    // See
    // https://docs.microsoft.com/en-us/previous-versions/visualstudio/visual-studio-2013/x99tb11d
    G3DCHECK(old_locale_.find(';') == std::string::npos)
        << "Old locale uses multiple locales per category, need to implement "
           "locale restore.";

    if (!new_locale_.empty()) {
      G3CHECK(!!std::setlocale(underlying_cast(category_), old_locale_.c_str()))
          << "Unable to restore old locale " << old_locale_ << " for category "
          << category_;
      new_locale_.clear();
    }
  }

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedProcessLocale);

  /**
   * Gets current locale.
   * @return optional with current locale, nullopt when no current locale found.
   */
  [[nodiscard]] std::optional<std::string> GetCurrentLocale() const noexcept {
    return !new_locale_.empty()
               ? std::optional{new_locale_}
               : (!old_locale_.empty() ? std::optional{old_locale_}
                                       : std::nullopt);
  }

 private:
  /**
   * Old and new locale string.
   */
  std::string old_locale_, new_locale_;
  /**
   * Locale category.
   */
  ScopedProcessLocaleCategory category_;
  /**
   * Explicit padding.
   */
  WB_ATTRIBUTE_UNUSED_FIELD std::byte
      pad_[sizeof(new_locale_) -  // NOLINT(bugprone-sizeof-container)
           sizeof(category_)]{};  //-V730_NOINIT

  /**
   * Sets locale.
   * @return New locale string.  Empty if locale not set.
   */
  [[nodiscard]] static std::string SetLocale(
      ScopedProcessLocaleCategory category, const char *new_locale) noexcept {
    const char *locale{std::setlocale(underlying_cast(category), new_locale)};
    G3DCHECK(locale == nullptr || locale[0] != '\0')
        << "std::setlocale returned empty string, can't distinguish it and "
           "error case.  Please, use some other error marker.";
    return locale ? locale : "";
  }
};

}  // namespace wb::base::intl

/**
 * operator << for ScopedProcessLocaleCategory.
 * @param s Stream.
 * @param category Category.
 * @return Stream with dumped category.
 */
inline std::basic_ostream<char, std::char_traits<char>> &operator<<(
    std::basic_ostream<char, std::char_traits<char>> &s,
    wb::base::intl::ScopedProcessLocaleCategory category) {
  if (category == wb::base::intl::ScopedProcessLocaleCategory::kAll) {
    return s << "All";
  }

  if (category == wb::base::intl::ScopedProcessLocaleCategory::kCollate) {
    return s << "Collate";
  }

  if (category == wb::base::intl::ScopedProcessLocaleCategory::kCharacterType) {
    return s << "Character Type";
  }

#ifdef WB_LOCALE_HAS_MESSAGES_CATEGORY
  if (category == wb::base::intl::ScopedProcessLocaleCategory::kMessages) {
    return s << "Messages";
  }
#endif

  if (category == wb::base::intl::ScopedProcessLocaleCategory::kMonetary) {
    return s << "Monetary";
  }

  if (category == wb::base::intl::ScopedProcessLocaleCategory::kNumeric) {
    return s << "Numeric";
  }

  if (category == wb::base::intl::ScopedProcessLocaleCategory::kTime) {
    return s << "Time";
  }

  return s << "<Unknown locale category: "
           << wb::base::underlying_cast(category) << ">";
}

#endif  // !WB_BASE_INTL_SCOPED_PROCESS_LOCALE_H_
