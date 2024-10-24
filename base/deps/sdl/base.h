// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// SDL base defines.

#ifndef WB_BASE_DEPS_SDL_BASE_H_
#define WB_BASE_DEPS_SDL_BASE_H_

#include <cerrno>
#include <ostream>
#include <sstream>
#include <string>
#include <variant>

#include "base/deps/fmt/format.h"
#include "base/deps/sdl/sdl.h"
#include "base/macroses.h"
#include "base/std2/system_error_ext.h"

namespace wb::sdl {

/**
 * @brief SDL error.
 */
struct error {
  /**
   * Creates SDL error from SDL return code.
   * @param rc SDL API return code.
   * @return SDL error.
   */
  [[nodiscard]] static error FromReturnCode(int rc) noexcept {
    std::string message{rc == 0 ? "" : ::SDL_GetError()};
    std::error_code code{!message.empty()
                             ? base::std2::posix_last_error_code(rc)
                             : base::std2::ok_code};
    return error{.message = std::move(message), .code = code};
  }

  /**
   * Creates SDL error from SDL bool return code.
   * @param rc SDL API bool return code.
   * @return SDL error.
   */
  [[nodiscard]] static error FromReturnBool(bool rc) noexcept {
    std::string message{rc ? "" : ::SDL_GetError()};
    std::error_code code{!message.empty()
                             ? base::std2::posix_last_error_code(EINVAL)
                             : base::std2::ok_code};
    return error{.message = std::move(message), .code = code};
  }

  /**
   * @brief Creates successful SDL result.
   * @return SDL error.
   */
  [[nodiscard]] static error Success() noexcept { return error{}; }

  /**
   * @brief Creates failed SDL result from last SDL error.  If no last error,
   * synthetic one is returned.
   * @param rc Return code.
   * @param message Message.
   * @return SDL error.
   */
  [[nodiscard]] static error Failure(
      int rc = EINVAL, std::string_view message = ::SDL_GetError()) noexcept {
    return error{.message = !message.empty() ? std::string{message}
                                             : "N/A <missed SDL error>",
                 .code = base::std2::posix_last_error_code(rc)};
  }

  /**
   * @brief Error message.
   */
  std::string message;

  /**
   * @brief Error code.
   */
  std::error_code code;

  /**
   * @brief Is succeeded?
   * @return true if not error, false otherwise.
   */
  [[nodiscard]] constexpr bool is_succeeded() const noexcept {
    return message.empty();
  }

  /**
   * @brief Is error?
   * @return true if error, false otherwise.
   */
  [[nodiscard]] constexpr bool is_failed() const noexcept {
    return !is_succeeded();
  }
};

/**
 * @brief Allows to use operator << for streaming error.
 * @param s Stream.
 * @param error error.
 * @return Stream.
 */
inline auto& operator<<(std::basic_ostream<char, std::char_traits<char>>& s,
                        const error& error) {
  return s << (error.is_failed() ? error.message : "");
}

/**
 * @brief SDL result.
 */
template <typename TResult>
using result = std::variant<TResult, error>;

/**
 * @brief Get error code from SDL result.
 * @tparam TResult SDL result success.
 * @param rc SDL result.
 * @return error code pointer or nullptr.
 */
template <typename TResult>
[[nodiscard]] constexpr const error* get_error(
    const result<TResult>& rc) noexcept {
  return std::get_if<error>(&rc);
}

/**
 * @brief Get success result from SDL result.
 * @tparam TResult TResult SDL result success.
 * @param rc SDL result.
 * @return SDL result success pointer or nullptr.
 */
template <typename TResult>
[[nodiscard]] constexpr TResult* get_result(result<TResult>& rc) noexcept {
  return std::get_if<TResult>(&rc);
}

/**
 * @brief Get success result from SDL result.
 * @tparam TResult TResult SDL result success.
 * @param rc SDL result.
 * @return SDL result success pointer or nullptr.
 */
template <typename TResult>
[[nodiscard]] constexpr const TResult* get_result(
    const result<TResult>& rc) noexcept {
  return std::get_if<TResult>(&rc);
}

}  // namespace wb::sdl

FMT_BEGIN_NAMESPACE
/**
 * @brief Fmt formatter for wb::sdl::error.
 */
template <>
struct formatter<wb::sdl::error> : formatter<std::string> {
  template <typename FormatContext>
  auto format(const wb::sdl::error& error, FormatContext& ctx) const {
    std::stringstream s{std::ios_base::out};
    s << error;
    return fmt::formatter<std::string>::format(s.str(), ctx);
  }
};
FMT_END_NAMESPACE

#endif  // !WB_BASE_DEPS_SDL_BASE_H_
