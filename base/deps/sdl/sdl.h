// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// SDL main header wrapper.

#ifndef WHITEBOX_BASE_DEPS_SDL_SDL_H_
#define WHITEBOX_BASE_DEPS_SDL_SDL_H_

#include "base/deps/sdl/sdl_config.h"
//
WB_BEGIN_SDL_WARNING_OVERRIDE_SCOPE()
#include "deps/sdl/include/SDL.h"
WB_END_SDL_WARNING_OVERRIDE_SCOPE()

#include "base/deps/sdl/sdl_init.h"
#include "base/deps/sdl/sdl_message_box.h"
#include "base/deps/sdl/sdl_version.h"
#include "base/deps/sdl/sdl_window.h"

#endif  // !WHITEBOX_BASE_DEPS_SDL_SDL_H_
