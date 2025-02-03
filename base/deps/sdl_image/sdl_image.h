// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// SDL_image wrapper.

#ifndef WB_BASE_DEPS_SDL_IMAGE_SDL_IMAGE_H_
#define WB_BASE_DEPS_SDL_IMAGE_SDL_IMAGE_H_

#include "base/deps/sdl_image/config.h"

WB_BEGIN_SDL_IMAGE_WARNING_OVERRIDE_SCOPE()
#include "deps/sdl_image/include/SDL3_image/SDL_image.h"
WB_END_SDL_IMAGE_WARNING_OVERRIDE_SCOPE()

#endif  // !WB_BASE_DEPS_SDL_IMAGE_SDL_IMAGE_H_
