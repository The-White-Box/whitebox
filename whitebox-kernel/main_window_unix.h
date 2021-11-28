// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// App main window on *nix.

#ifndef WB_WHITEBOX_KERNEL_MAIN_WINDOW_UNIX_H_
#define WB_WHITEBOX_KERNEL_MAIN_WINDOW_UNIX_H_

#include <utility>  // std::move

#include "base/base_macroses.h"
#include "base/deps/sdl/window.h"

namespace wb::kernel {

/**
 * Main window via SDL.
 */
class MainWindow {
 public:
  /**
   * @brief Create main window.
   * @param title Title.
   * @param width Width.
   * @param height Height.
   * @param window_flags Window flags.
   * @return Main window.
   */
  [[nodiscard]] static sdl::result<MainWindow> New(
      const char* title, int width, int height,
      sdl::WindowFlags window_flags) noexcept;

  /**
   * Move constructor.
   * @param w Window to move construct from.
   */
  MainWindow(MainWindow&& w) noexcept : window_{std::move(w.window_)} {}

  /**
   * Move assignment operator.
   * @param w Window to move assign from.
   */
  MainWindow& operator=(MainWindow&& w) noexcept {
    std::swap(window_, w.window_);
    return *this;
  }

  /**
   * Get underlying window UI platform info.
   * @param platform_info Platform info.
   * @return error if any.
   */
  [[nodiscard]] sdl::error GetPlatformInfo(
      ::SDL_SysWMinfo& platform_info) const noexcept {
    return window_.GetPlatformInfo(platform_info);
  }

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(MainWindow);

 private:
  /**
   * SDL window.
   */
  sdl::Window window_;

  /**
   * Creates main window.
   * @param window SDL window.
   */
  explicit MainWindow(sdl::Window&& window) noexcept
      : window_{std::move(window)} {}
};

}  // namespace wb::kernel

#endif  // !WB_WHITEBOX_KERNEL_MAIN_WINDOW_UNIX_H_
