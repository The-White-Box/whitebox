// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// SDL base defines.

#ifndef WB_BASE_DEPS_SDL_BASE_H_
#define WB_BASE_DEPS_SDL_BASE_H_

#include <ostream>
#include <sstream>
#include <string>
#include <variant>

#include "base/base_macroses.h"
#include "base/deps/fmt/format.h"
#include "base/deps/sdl/sdl.h"

namespace wb::sdl {
/**
 * @brief SDL error.
 */
struct SdlError {
  /**
   * Creates SDL error from SDL return code.
   * @param rc SDL API return code.
   * @return SDL error.
   */
  [[nodiscard]] static SdlError FromReturnCode(int rc) noexcept {
    return SdlError{.message = rc == 0 ? nullptr : ::SDL_GetError()};
  }

  /**
   * Creates SDL error from SDL bool return code.
   * @param rc SDL API bool return code.
   * @return SDL error.
   */
  [[nodiscard]] static SdlError FromReturnBool(SDL_bool rc) noexcept {
    return SdlError{.message = rc == SDL_TRUE ? nullptr : ::SDL_GetError()};
  }

  /**
   * @brief Creates successful SDL result.
   * @return SDL error.
   */
  static SdlError Success() noexcept { return SdlError{}; }

  /**
   * @brief Creates failed SDL result from last SDL error.  If no last error,
   * synthetic one is returned.
   * @return SDL error.
   */
  static SdlError Failure(const char* message = ::SDL_GetError()) noexcept {
    return SdlError{
        .message = message && message[0] ? message : "N/A <missed SDL error>"};
  }

  /**
   * @brief Error message.
   */
  const char* message;

  /**
   * @brief Is succeeded?
   * @return true if not error, false otherwise.
   */
  [[nodiscard]] constexpr bool IsSucceeded() const noexcept {
    return message == nullptr || !message[0];
  }

  /**
   * @brief Is error?
   * @return true if error, false otherwise.
   */
  [[nodiscard]] constexpr bool IsFailed() const noexcept {
    return !IsSucceeded();
  }
};

/**
 * @brief Allows to use operator << for streaming SdlError.
 * @param s Stream.
 * @param error SdlError.
 * @return Stream.
 */
inline auto& operator<<(std::basic_ostream<char, std::char_traits<char>>& s,
                        const SdlError& error) {
  return s << (error.IsFailed() ? error.message : "");
}

/**
 * @brief SDL result.
 */
template <typename TResult>
using SdlResult = std::variant<TResult, SdlError>;

/**
 * @brief Get error code from SDL result.
 * @tparam TResult SDL result success.
 * @param rc SDL result.
 * @return error code pointer or nullptr.
 */
template <typename TResult>
[[nodiscard]] constexpr const SdlError* GetError(
    const SdlResult<TResult>& rc) noexcept {
  return std::get_if<SdlError>(&rc);
}

/**
 * @brief Get success result from SDL result.
 * @tparam TResult TResult SDL result success.
 * @param rc SDL result.
 * @return SDL result success pointer or nullptr.
 */
template <typename TResult>
[[nodiscard]] constexpr TResult* GetSuccessResult(
    SdlResult<TResult>& rc) noexcept {
  return std::get_if<TResult>(&rc);
}

/**
 * @brief Get success result from SDL result.
 * @tparam TResult TResult SDL result success.
 * @param rc SDL result.
 * @return SDL result success pointer or nullptr.
 */
template <typename TResult>
[[nodiscard]] constexpr const TResult* GetSuccessResult(
    const SdlResult<TResult>& rc) noexcept {
  return std::get_if<TResult>(&rc);
}
}  // namespace wb::sdl

FMT_BEGIN_NAMESPACE
template <>
struct formatter<wb::sdl::SdlError> : formatter<std::string> {
  template <typename FormatContext>
  auto format(const wb::sdl::SdlError& error, FormatContext& ctx) {
    std::stringstream s{std::ios_base::out};
    s << error;
    return fmt::formatter<std::string>::format(s.str(), ctx);
  }
};
FMT_END_NAMESPACE

#endif  // !WB_BASE_DEPS_SDL_BASE_H_
