// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Provides an API for looking up localized message strings.

#include "base/intl/lookup.h"

#include <map>
#include <unordered_map>

#include "base/deps/g3log/g3log.h"
#include "base/intl/message_ids.h"

namespace wb::base::intl {
/**
 * @brief Lookup implementation.
 */
class Lookup::LookupImpl {
 public:
  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(LookupImpl);

  [[nodiscard]] static Lookup::LookupResult<un<Lookup::LookupImpl>> New(
      const std::set<std::string>& locale_ids) noexcept {
    // TODO(dimhotepus): Load locales.
    if (locale_ids.find("English_United States.utf8") != locale_ids.end() ||
        locale_ids.find("en_US.UTF-8") != locale_ids.end()) {
      return Lookup::LookupResult<un<Lookup::LookupImpl>>{un<
          Lookup::LookupImpl>{new Lookup::LookupImpl{MessagesById{
#ifdef WB_OS_WIN
          {message_ids::kWindowsVersionIsTooOld,
           "Windows is too old.  At least Windows 10, version 1903 (May 19, "
           "2019)+ required."},
          {message_ids::kPleaseUpdateWindowsVersion,
           "Please, update Windows to Windows 10, version 1903 (May 19, 2019) "
           "or greater."},
          {message_ids::kBootmgrErrorDialogTitle, "Boot Manager - Error"},
          {message_ids::kSeeTechnicalDetails, "See techical details"},
          {message_ids::kHideTechnicalDetails, "Hide techical details"},
#endif
          {message_ids::kNudgeAuthorsLink,
           "<A "
           "HREF=\"https://github.com/The-White-Box/whitebox/issues\">Nudge</"
           "A> "
           "authors"},
          {message_ids::kCantGetExecutableDirectoryForBootManager,
           "Can't get executable directory.  Unable to load the kernel."},
          {message_ids::kPleaseNudgeAuthors, "Please, nudge authors."},
          {message_ids::kCantGetKernelEntryPoint,
           "Can't get '{0}' entry point from '{1}' kernel."},
          {message_ids::kPleaseReinstallTheGame,
           "Looks like app is broken, please, reinstall the one."},
          {message_ids::kCantLoadKernelFrom,
           "Can't load whitebox kernel '{0}'."},
      }}}};
    }

    return Lookup::LookupResult<un<Lookup::LookupImpl>>{
        Lookup::Status::kArgumentError};
  }

  [[nodiscard]] Lookup::LookupResult<Lookup::ref<const std::string>> String(
      uint64_t message_id) noexcept {
    if (const auto& message = messages_by_id_.find(message_id);
        message != messages_by_id_.end()) [[likely]] {
      return Lookup::LookupResult<Lookup::ref<const std::string>>{
          std::ref(message->second)};
    }

    G3LOG(WARNING) << "Missed localization string for " << message_id
                   << " message id.";
    return Lookup::LookupResult<Lookup::ref<const std::string>>{
        Lookup::Status::kUnavailable};
  }

 private:
  using MessagesById = std::unordered_map<uint64_t, std::string>;

  const MessagesById messages_by_id_;

  /**
   * @brief Creates lookup implementation.
   * @param messages_by_id Messages by ids map.
   * @return nothing.
   */
  LookupImpl(MessagesById messages_by_id) noexcept
      : messages_by_id_{std::move(messages_by_id)} {}
};

[[nodiscard]] Lookup::LookupResult<Lookup> Lookup::New(
    const std::set<std::string>& locale_ids) noexcept {
  auto impl_result = LookupImpl::New(locale_ids);
  if (auto* impl = std::get_if<un<LookupImpl>>(&impl_result)) [[likely]] {
    return Lookup::LookupResult<Lookup>{Lookup{std::move(*impl)}};
  }

  return Lookup::LookupResult<Lookup>{std::get<Lookup::Status>(impl_result)};
}

[[nodiscard]] Lookup::LookupResult<Lookup::ref<const std::string>>
Lookup::String(uint64_t message_id) noexcept {
  G3DCHECK(!!impl_);
  return impl_->String(message_id);
}

[[nodiscard]] Lookup::LookupResult<std::string> Lookup::StringFormat(
    uint64_t message_id, fmt::format_args format_args) noexcept {
  auto result = String(message_id);
  if (const auto* string = std::get_if<Lookup::ref<const std::string>>(&result))
      [[likely]] {
    return fmt::vformat(static_cast<const std::string&>(*string), format_args);
  }
  return Lookup::LookupResult<std::string>{std::get<Lookup::Status>(result)};
}

[[nodiscard]] Lookup::StringLayout Lookup::Layout() const noexcept {
  // TODO(dimhotepus): Handle Right to Left messages.
  return Lookup::StringLayout::LeftToRight;
}

Lookup::Lookup(un<LookupImpl> impl) noexcept : impl_{std::move(impl)} {}

Lookup::~Lookup() noexcept = default;

Lookup::Lookup(Lookup&& l) noexcept : impl_{std::move(l.impl_)} {}

LookupWithFallback::LookupWithFallback(LookupWithFallback&& l) noexcept
    : lookup_{std::move(l.lookup_)} {}

[[nodiscard]] Lookup::LookupResult<LookupWithFallback> LookupWithFallback::New(
    const std::set<std::string>& locale_ids,
    std::string fallback_string) noexcept {
  auto lookup_result = Lookup::New(locale_ids);
  if (auto* lookup = std::get_if<Lookup>(&lookup_result)) [[likely]] {
    return Lookup::LookupResult<LookupWithFallback>{
        LookupWithFallback(std::move(*lookup), std::move(fallback_string))};
  }

  return Lookup::LookupResult<LookupWithFallback>{
      std::get<Lookup::Status>(lookup_result)};
}

[[nodiscard]] const std::string& LookupWithFallback::String(
    uint64_t message_id) noexcept {
  auto result = lookup_.String(message_id);
  if (const auto* string = std::get_if<Lookup::ref<const std::string>>(&result))
      [[likely]] {
    return *string;
  }

  G3LOG(WARNING) << "Missed localization string for " << message_id
                 << " message id.";
  return fallback_string_;
}

[[nodiscard]] std::string LookupWithFallback::StringFormat(
    uint64_t message_id, fmt::format_args format_args) noexcept {
  auto result = lookup_.String(message_id);
  if (const auto* string = std::get_if<Lookup::ref<const std::string>>(&result))
      [[likely]] {
    return fmt::vformat(static_cast<const std::string&>(*string), format_args);
  }

  G3LOG(WARNING) << "Missed localization string for " << message_id
                 << " message id.";
  return fallback_string_;
}

[[nodiscard]] Lookup::StringLayout LookupWithFallback::Layout() const noexcept {
  return lookup_.Layout();
}

LookupWithFallback::LookupWithFallback(Lookup lookup,
                                       std::string fallback_string) noexcept
    : lookup_{std::move(lookup)},
      fallback_string_{std::move(fallback_string)} {}
}  // namespace wb::base::intl