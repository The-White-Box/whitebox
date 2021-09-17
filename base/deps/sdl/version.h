// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// SDL version wrapper.

#ifndef WB_BASE_DEPS_SDL_VERSION_H_
#define WB_BASE_DEPS_SDL_VERSION_H_

#include <ostream>

#include "base/deps/sdl/sdl.h"
#include "base/std2/cstring_ext.h"

/**
 * @brief Streams SDL_version via <<.
 * @param s Stream.
 * @param version SDL_version.
 * @return Stream.
 */
inline std::basic_ostream<char, std::char_traits<char>>& operator<<(
    std::basic_ostream<char, std::char_traits<char>>& s,
    const SDL_version& version) {
  return s << wb::base::implicit_cast<unsigned>(version.major) << '.'
           << wb::base::implicit_cast<unsigned>(version.minor) << '.'
           << wb::base::implicit_cast<unsigned>(version.patch);
}

namespace wb::sdl {
/**
 * Get SDL version with which app is compiled (not runtime one!).
 * @return Compile time SDL version.
 */
inline SDL_version GetCompileTimeVersion() noexcept {
  ::SDL_version version;
  base::std2::BitwiseMemset(version, 0);

  SDL_VERSION(&version);

  static_assert(sizeof(SDL_version) <= 8U,
                "SDL_version becomes too huge to ensure pass by value, use "
                "pass by reference instead.");
  return version;
}

/**
 * Get SDL version with which app is linked (runtime one!).
 * @return Link time SDL version.
 */
inline SDL_version GetLinkTimeVersion() noexcept {
  ::SDL_version version;
  base::std2::BitwiseMemset(version, 0);

  ::SDL_GetVersion(&version);

  static_assert(sizeof(SDL_version) <= 8U,
                "SDL_version becomes too huge to ensure pass by value, use "
                "pass by reference instead.");
  return version;
}
}  // namespace wb::sdl

#endif  // !WB_BASE_DEPS_SDL_VERSION_H_
