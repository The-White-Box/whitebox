// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// SDL_syswm.h wrapper.

#ifndef WB_BASE_DEPS_SDL_SYSWM_H_
#define WB_BASE_DEPS_SDL_SYSWM_H_

#include <ostream>

#include "base/macroses.h"
#include "base/deps/g3log/g3log.h"
#include "base/deps/sdl/config.h"
//
WB_BEGIN_SDL_WARNING_OVERRIDE_SCOPE()
#include "deps/sdl/include/SDL_syswm.h"
WB_END_SDL_WARNING_OVERRIDE_SCOPE()

/**
 * @brief Allows to use operator << for streaming SDL_SYSWM_TYPE.
 * @param s Stream.
 * @param error error.
 * @return Stream.
 */
inline auto& operator<<(std::basic_ostream<char, std::char_traits<char>>& s,
                        SDL_SYSWM_TYPE type) {
  switch (type) {
    case SDL_SYSWM_UNKNOWN:
      return s << "Unknown";
    case SDL_SYSWM_WINDOWS:
      return s << "Windows";
    case SDL_SYSWM_X11:
      return s << "X11";
    case SDL_SYSWM_DIRECTFB:
      return s << "DirectFB";
    case SDL_SYSWM_COCOA:
      return s << "Cocoa";
    case SDL_SYSWM_UIKIT:
      return s << "UiKit";
    case SDL_SYSWM_WAYLAND:
      return s << "Wayland";
    /* no longer available, left for API/ABI compatibility.  Remove in 2.1! */
    case SDL_SYSWM_MIR:
      return s << "Mir";
    case SDL_SYSWM_WINRT:
      return s << "WinRT";
    case SDL_SYSWM_ANDROID:
      return s << "Android";
    case SDL_SYSWM_VIVANTE:
      return s << "Vivante";
    case SDL_SYSWM_OS2:
      return s << "OS2";
    case SDL_SYSWM_HAIKU:
      return s << "Haiku";
    case SDL_SYSWM_KMSDRM:
      return s << "DRM/KMS";
    case SDL_SYSWM_RISCOS:
      return s << "RiscOS";
    default:
      // type != SDL_SYSWM_UNKNOWN is always true,  This is used to prevent
      // compiler from complaining about constant in if condition.
      G3DCHECK(type != SDL_SYSWM_UNKNOWN) //-V547
          << "Unknown window subsystem ("
          << wb::base::underlying_cast(type) << "), assume Unknown.";
      return s << "Unknown";
  }
}

#endif  // !WB_BASE_DEPS_SDL_SYSWM_H_
