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

  [[nodiscard]] static LookupResult<un<Lookup::LookupImpl>> New(
      const std::set<std::string>& locale_ids) noexcept {
    // TODO(dimhotepus): Load locales.
    if (locale_ids.find("English_United States.utf8") != locale_ids.end() ||
        locale_ids.find("en_US.UTF-8") != locale_ids.end()) {
      return LookupResult<un<Lookup::LookupImpl>>{un<
          Lookup::LookupImpl>{new Lookup::LookupImpl{MessagesById{
#ifdef WB_OS_WIN
          {message_ids::kWindowsVersionIsTooOld,
           "Windows is too old.  At least Windows 10, version 1903 (May 19, "
           "2019)+ required."},
          {message_ids::kPleaseUpdateWindowsVersion,
           "Please, update Windows to Windows 10, version 1903 (May 19, 2019) "
           "or greater."},
          {message_ids::kSeeTechnicalDetails, "See techical details"},
          {message_ids::kHideTechnicalDetails, "Hide techical details"},
#endif
          {message_ids::kBootmgrErrorDialogTitle, "Boot Manager - Error"},
          {message_ids::kNudgeAuthorsLink,
           "<A "
           "HREF=\"https://github.com/The-White-Box/whitebox/issues\">Nudge</"
           "A> "
           "authors"},
          {message_ids::kCantGetExecutableDirectoryForBootManager,
           "Can't get executable directory.  Unable to load the kernel."},
          {message_ids::kCantGetLibraryEntryPoint,
           "Can't get '{0}' entry point from '{1}'."},
          {message_ids::kPleaseReinstallTheGame,
           "Looks like app is broken, please, reinstall the one."},
          {message_ids::kCantLoadKernelFrom,
           "Can't load whitebox kernel '{0}'."},
          {message_ids::kAppErrorDialogTitle, "{0} - Error"},
          {
              message_ids::kPleaseCheckAppInstalledCorrectly,
              "Please, check app is installed correctly and you have enough "
              "permissions to run it.",
          },
          {
              message_ids::kCantGetCurrentDirectoryUnableToLoadTheApp,
              "Can't get current directory.  Unable to load the app.",
          },
          {
              message_ids::kCantLoadBootManager,
              "Can't load boot manager '{0}'.",
          },
          {
              message_ids::kKernelErrorDialogTitle,
              "Whitebox Kernel - Error",
          },
          {
              message_ids::kPleaseCheckMouseOnYourDevice,
              "Please, check mouse is connected and working.",
          },
          {
              message_ids::kUnableToRegisterMouseDevice,
              "Unable to register mouse as <A "
              "HREF=\"https://docs.microsoft.com/en-us/windows/win32/inputdev/"
              "about-raw-input\">Raw Input</A> device.",
          },
          {
              message_ids::kPleaseCheckKeyboardOnYourDevice,
              "Please, check keyboard is connected and working.",
          },
          {
              message_ids::kUnableToRegisterKeyboardDevice,
              "Unable to register keyboard as <A "
              "HREF=\"https://docs.microsoft.com/en-us/windows/win32/inputdev/"
              "about-raw-input\">Raw Input</A> device.",
          },
      }}}};
    }

    return LookupResult<un<Lookup::LookupImpl>>{Status::kArgumentError};
  }

  [[nodiscard]] LookupResult<Lookup::Ref<const std::string>> String(
      uint64_t message_id) const noexcept {
    if (const auto& message = messages_by_id_.find(message_id);
        message != messages_by_id_.end())
      WB_ATTRIBUTE_LIKELY {
        return LookupResult<Lookup::Ref<const std::string>>{
            std::ref(message->second)};
      }

    G3LOG(WARNING) << "Missed localization string for " << message_id
                   << " message id.";
    return LookupResult<Lookup::Ref<const std::string>>{Status::kUnavailable};
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

[[nodiscard]] LookupResult<Lookup> Lookup::New(
    const std::set<std::string>& locale_ids) noexcept {
  auto impl_result = LookupImpl::New(locale_ids);
  if (auto* impl = std::get_if<un<LookupImpl>>(&impl_result))
    WB_ATTRIBUTE_LIKELY {
      return LookupResult<Lookup>{Lookup{std::move(*impl)}};
    }

  return LookupResult<Lookup>{std::get<Status>(impl_result)};
}

[[nodiscard]] LookupResult<Lookup::Ref<const std::string>> Lookup::String(
    uint64_t message_id) const noexcept {
  G3DCHECK(!!impl_);
  return impl_->String(message_id);
}

[[nodiscard]] LookupResult<std::string> Lookup::Format(
    uint64_t message_id, fmt::format_args format_args) const noexcept {
  auto result = String(message_id);
  if (const auto* string = std::get_if<Lookup::Ref<const std::string>>(&result))
    WB_ATTRIBUTE_LIKELY {
      return fmt::vformat(static_cast<const std::string&>(*string),
                          format_args);
    }
  return LookupResult<std::string>{std::get<Status>(result)};
}

[[nodiscard]] WB_ATTRIBUTE_CONST StringLayout Lookup::Layout() const noexcept {
  // TODO(dimhotepus): Handle Right to Left messages.
  return StringLayout::LeftToRight;
}

Lookup::Lookup(un<LookupImpl> impl) noexcept : impl_{std::move(impl)} {}

Lookup::~Lookup() noexcept = default;

Lookup::Lookup(Lookup&& l) noexcept : impl_{std::move(l.impl_)} {}

LookupWithFallback::LookupWithFallback(LookupWithFallback&& l) noexcept
    : lookup_{std::move(l.lookup_)} {}

[[nodiscard]] LookupResult<LookupWithFallback> LookupWithFallback::New(
    const std::set<std::string>& locale_ids,
    std::string fallback_string) noexcept {
  auto lookup_result = Lookup::New(locale_ids);
  if (auto* lookup = std::get_if<Lookup>(&lookup_result)) WB_ATTRIBUTE_LIKELY {
      return LookupResult<LookupWithFallback>{
          LookupWithFallback(std::move(*lookup), std::move(fallback_string))};
    }

  return LookupResult<LookupWithFallback>{std::get<Status>(lookup_result)};
}

[[nodiscard]] const std::string& LookupWithFallback::String(
    uint64_t message_id) const noexcept {
  auto result = lookup_.String(message_id);
  if (const auto* string = std::get_if<Lookup::Ref<const std::string>>(&result))
    WB_ATTRIBUTE_LIKELY { return *string; }

  G3LOG(WARNING) << "Missed localization string for " << message_id
                 << " message id.";
  return fallback_string_;
}

[[nodiscard]] std::string LookupWithFallback::Format(
    uint64_t message_id, fmt::format_args format_args) const noexcept {
  auto result = lookup_.String(message_id);
  if (const auto* string = std::get_if<Lookup::Ref<const std::string>>(&result))
    WB_ATTRIBUTE_LIKELY {
      return fmt::vformat(static_cast<const std::string&>(*string),
                          format_args);
    }

  G3LOG(WARNING) << "Missed localization string for " << message_id
                 << " message id.";
  return fallback_string_;
}

[[nodiscard]] WB_ATTRIBUTE_CONST StringLayout
LookupWithFallback::Layout() const noexcept {
  return lookup_.Layout();
}

LookupWithFallback::LookupWithFallback(Lookup lookup,
                                       std::string fallback_string) noexcept
    : lookup_{std::move(lookup)},
      fallback_string_{std::move(fallback_string)} {}
}  // namespace wb::base::intl
