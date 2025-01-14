// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// SDL image configuration.

#ifndef WB_BASE_DEPS_SDL_IMAGE_CONFIG_H_
#define WB_BASE_DEPS_SDL_IMAGE_CONFIG_H_

#include "build/compiler_config.h"

/**
 * @brief Open SDL compiler warnings suppression scope.
 */
#define WB_BEGIN_SDL_IMAGE_WARNING_OVERRIDE_SCOPE() \
  WB_GCC_BEGIN_WARNING_OVERRIDE_SCOPE()       \
    WB_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()    \
      WB_MSVC_DISABLE_WARNING(4820)           \
      WB_GCC_DISABLE_OLD_STYLE_CAST_WARNING() \
      WB_GCC_DISABLE_PADDED_WARNING()         \
      WB_GCC_DISABLE_SWITCH_DEFAULT_WARNING()

/**
 * @brief Close SDL compiler warnings suppression scope.
 */
#define WB_END_SDL_IMAGE_WARNING_OVERRIDE_SCOPE() \
  WB_MSVC_END_WARNING_OVERRIDE_SCOPE        \
  ()                                        \
  WB_GCC_END_WARNING_OVERRIDE_SCOPE         \
  ()

#endif  // !WB_BASE_DEPS_SDL_IMAGE_CONFIG_H_
