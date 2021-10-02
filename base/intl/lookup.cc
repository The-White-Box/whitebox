// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Provides an API for looking up localized message strings.

#include "base/intl/lookup.h"

#include <map>
#include <unordered_map>

#include "base/deps/g3log/g3log.h"

namespace {

/**
 * Computes hash for |string|.
 * @param string String to hash.
 * @return String hash.
 */
[[nodiscard]] WB_ATTRIBUTE_CONST WB_ATTRIBUTE_FORCEINLINE constexpr uint64_t
hash(std::string_view string) noexcept {
  return wb::base::intl::I18nStringViewHash{}(string);
}

}  // namespace

namespace wb::base::intl {

/**
 * @brief Lookup implementation.
 */
class Lookup::LookupImpl {
 public:
  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(LookupImpl);

  WB_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()
    // C4868 compiler may not enforce left-to-right evaluation order
    // in braced initializer list.  hash is pure function here.
    WB_MSVC_DISABLE_WARNING(4868)
    [[nodiscard]] static LookupResult<un<Lookup::LookupImpl>> New(
        const std::set<std::string_view>& locale_ids) noexcept {
      // TODO(dimhotepus): Load locales.
      if (locale_ids.find("English_United States.utf8") != locale_ids.end() ||
          locale_ids.find("en_US.UTF-8") != locale_ids.end()) {
        return LookupResult<un<Lookup::LookupImpl>>{un<
            Lookup::LookupImpl>{new Lookup::LookupImpl{
            MessagesById{
        //-V509,
#ifdef WB_OS_WIN
                {hash("Windows is too old.  At least Windows 10, version 1903 "
                      "(May "
                      "19, "
                      "2019)+ required."),
                 "Windows is too old.  At least Windows 10, version 1903 (May "
                 "19, "
                 "2019)+ required."},
                {hash("Please, update Windows to Windows 10, version 1903 (May "
                      "19, "
                      "2019) or greater."),
                 "Please, update Windows to Windows 10, version 1903 (May 19, "
                 "2019) "
                 "or greater."},
                {hash("See techical details"), "See techical details"},
                {hash("Hide techical details"), "Hide techical details"},
#endif
                {hash("Boot Manager - Error"), "Boot Manager - Error"},
                {hash("<A "
                      "HREF=\"https://github.com/The-White-Box/whitebox/"
                      "issues\">Nudge</"
                      "A> "
                      "authors"),
                 "<A "
                 "HREF=\"https://github.com/The-White-Box/whitebox/"
                 "issues\">Nudge</"
                 "A> "
                 "authors"},
                {hash("Can't get executable directory.  Unable to load the "
                      "kernel."),
                 "Can't get executable directory.  Unable to load the kernel."},
                {hash("Can't get '{0}' entry point from '{1}'."),
                 "Can't get '{0}' entry point from '{1}'."},
                {hash("Can't load whitebox kernel '{0}'."),
                 "Can't load whitebox kernel '{0}'."},
                {hash("{0} - Error"), "{0} - Error"},
                {
                    hash(
                        "Please, check app is installed correctly and you have "
                        "enough permissions to run it."),
                    "Please, check app is installed correctly and you have "
                    "enough "
                    "permissions to run it.",
                },
                {
                    hash("Can't get current directory.  May be app located too "
                         "deep (> 1024)?"),
                    "Can't get current directory.  May be app located too deep "
                    "(> 1024)?",
                },
                {
                    hash("Can't get current directory.  Unable to load the "
                         "kernel."),
                    "Can't get current directory.  Unable to load the kernel.",
                },
                {
                    hash("Can't load boot manager '{0}'."),
                    "Can't load boot manager '{0}'.",
                },
                {
                    hash("Whitebox Kernel - Error"),
                    "Whitebox Kernel - Error",
                },
                {
                    hash("Please, check mouse is connected and working."),
                    "Please, check mouse is connected and working.",
                },
                {
                    hash(
                        "Unable to register mouse as <A "
                        "HREF=\"https://docs.microsoft.com/en-us/windows/win32/"
                        "inputdev/"
                        "about-raw-input\">Raw Input</A> device."),
                    "Unable to register mouse as <A "
                    "HREF=\"https://docs.microsoft.com/en-us/windows/win32/"
                    "inputdev/"
                    "about-raw-input\">Raw Input</A> device.",
                },
                {
                    hash("Please, check keyboard is connected and working."),
                    "Please, check keyboard is connected and working.",
                },
                {
                    hash(
                        "Unable to register keyboard as <A "
                        "HREF=\"https://docs.microsoft.com/en-us/windows/win32/"
                        "inputdev/"
                        "about-raw-input\">Raw Input</A> device."),
                    "Unable to register keyboard as <A "
                    "HREF=\"https://docs.microsoft.com/en-us/windows/win32/"
                    "inputdev/"
                    "about-raw-input\">Raw Input</A> device.",
                },
                {
                    hash("Please, check your SDL library installed and "
                         "working."),
                    "Please, check your SDL library installed and working.",
                },
                {
                    hash("SDL build/runtime v.{0}/v.{1}, revision '{2}' "
                         "initialization "
                         "failed.\n\n{3}."),
                    "SDL build/runtime v.{0}/v.{1}, revision '{2}' "
                    "initialization "
                    "failed.\n\n{3}.",
                },
                {hash("SDL image parser initialization failed for image types "
                      "{0}.\n\n{1}."),
                 "SDL image parser initialization failed for image types "
                 "{0}.\n\n{1}."},
                {hash("Please, check you installed '{0}' libraries/drivers."),
                 "Please, check you installed '{0}' libraries/drivers."},
                {hash("SDL window create failed with '{0}' context.\n\n{1}."),
                 "SDL window create failed with '{0}' context.\n\n{1}."}},
            StringLayout::LeftToRight}}};
      }

      return LookupResult<un<Lookup::LookupImpl>>{Status::kArgumentError};
    }
  WB_MSVC_END_WARNING_OVERRIDE_SCOPE()

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

  [[nodiscard]] WB_ATTRIBUTE_CONST StringLayout Layout() const noexcept {
    return string_layout_;
  }

 private:
  using MessagesById = std::unordered_map<uint64_t, std::string>;

  const MessagesById messages_by_id_;
  const StringLayout string_layout_;

  // NOLINTNEXTLINE(modernize-avoid-c-arrays)
  WB_ATTRIBUTE_UNUSED_FIELD std::byte
      pad_[sizeof(char*) - sizeof(string_layout_)];

  /**
   * @brief Creates lookup implementation.
   * @param messages_by_id Messages by ids map.
   * @return nothing.
   */
  LookupImpl(MessagesById messages_by_id, StringLayout string_layout) noexcept
      : messages_by_id_{std::move(messages_by_id)},
        string_layout_{string_layout} {}
};

[[nodiscard]] LookupResult<Lookup> Lookup::New(
    const std::set<std::string_view>& locale_ids) noexcept {
  auto impl_result = LookupImpl::New(locale_ids);
  if (auto* impl = std::get_if<un<LookupImpl>>(&impl_result))
    WB_ATTRIBUTE_LIKELY {
      return LookupResult<Lookup>{Lookup{std::move(*impl)}};
    }

  const auto* status = std::get_if<Status>(&impl_result);
  G3DCHECK(!!status);
  return LookupResult<Lookup>{*status};
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

  const auto* status = std::get_if<Status>(&result);
  G3DCHECK(!!status);
  return LookupResult<std::string>{*status};
}

[[nodiscard]] WB_ATTRIBUTE_CONST StringLayout Lookup::Layout() const noexcept {
  return impl_->Layout();
}

Lookup::Lookup(un<LookupImpl> impl) noexcept : impl_{std::move(impl)} {}

Lookup::~Lookup() noexcept = default;

Lookup::Lookup(Lookup&& l) noexcept : impl_{std::move(l.impl_)} {}

LookupWithFallback::LookupWithFallback(LookupWithFallback&& l) noexcept
    : lookup_{std::move(l.lookup_)},
      fallback_string_{std::move(l.fallback_string_)} {}

[[nodiscard]] LookupResult<LookupWithFallback> LookupWithFallback::New(
    const std::set<std::string_view>& locale_ids,
    std::string fallback_string) noexcept {
  auto lookup_result = Lookup::New(locale_ids);
  if (auto* lookup = std::get_if<Lookup>(&lookup_result)) WB_ATTRIBUTE_LIKELY {
      return LookupResult<LookupWithFallback>{
          LookupWithFallback(std::move(*lookup), std::move(fallback_string))};
    }

  const auto* status = std::get_if<Status>(&lookup_result);
  G3DCHECK(!!status);
  return LookupResult<LookupWithFallback>{*status};
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
  auto result = lookup_.Format(message_id, format_args);
  if (const auto* string = std::get_if<std::string>(&result))
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
