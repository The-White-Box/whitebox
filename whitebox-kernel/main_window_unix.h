// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// App main window on *nix.

#ifndef WB_WHITEBOX_KERNEL_MAIN_WINDOW_UNIX_H_
#define WB_WHITEBOX_KERNEL_MAIN_WINDOW_UNIX_H_

#include <string>

#include "base/deps/sdl/window.h"

namespace wb::kernel {
class MainWindow {
 public:
  [[nodiscard]] static sdl::SdlResult<MainWindow> New(const std::string &title,
                                                      int width,
                                                      int height) noexcept {
    auto sdl_window = sdl::SdlWindow::New(title.c_str(), SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED, width, height,
                                          GetWindowFlags());
    if (auto *window = sdl::get_result(sdl_window)) WB_ATTRIBUTE_LIKELY {
        return sdl::SdlResult<MainWindow>{MainWindow{std::move(*window)}};
      }
    return sdl::SdlResult<MainWindow>{*sdl::get_error(sdl_window)};
  }

 private:
  sdl::SdlWindow window_;

  explicit MainWindow(sdl::SdlWindow &&window) noexcept
      : window_{std::move(window)} {}

  static sdl::SdlWindowFlags GetWindowFlags() noexcept {
    // TODO(dimhotepus): kAllowHighDpi requires usage of SDL_GetWindowSize() to
    // query the client area's size in screen coordinates, and
    // SDL_GL_GetDrawableSize() or SDL_GetRendererOutputSize() to query the
    // drawable size in pixels.
    return sdl::SdlWindowFlags::kResizable | sdl::SdlWindowFlags::kAllowHighDpi
#if defined(WB_OS_LINUX)
           | sdl::SdlWindowFlags::kUseVulkan
#elif defined(WB_OS_MACOSX)
           | sdl::SdlWindowFlags::kUseMetal
#else
#error Unknown platform. Please, define SDL window flags for your platform in \
	whitebox-kernel/whitebox_kernel_main.cc
#endif
        ;
  }
};
}  // namespace wb::kernel

#endif  // !WB_WHITEBOX_KERNEL_MAIN_WINDOW_UNIX_H_
