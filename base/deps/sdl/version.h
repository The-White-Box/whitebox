// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// SDL version wrapper.

#ifndef WB_BASE_DEPS_SDL_VERSION_H_
#define WB_BASE_DEPS_SDL_VERSION_H_

#include "base/deps/sdl/sdl.h"

namespace wb::sdl {

/**
 * Get SDL version with which app is compiled (not runtime one!).
 * @return Compile time SDL version.
 */
[[nodiscard]] inline int GetCompileTimeVersion() noexcept {
  return SDL_VERSION;
}

/**
 * Get SDL version with which app is linked (runtime one!).
 * @return Link time SDL version.
 */
[[nodiscard]] inline int GetLinkTimeVersion() noexcept {
  return ::SDL_GetVersion();
}

}  // namespace wb::sdl

#endif  // !WB_BASE_DEPS_SDL_VERSION_H_
