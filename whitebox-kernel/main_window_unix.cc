// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// App main window on *nix.

#include "main_window_unix.h"

#include <string>

#include "build/static_settings_config.h"

namespace {

/**
 * @brief Make window icon name.
 * @param window_name Window name.
 * @return Window icon name.
 */
[[nodiscard]] std::string MakeWindowIconName(
    const std::string& window_name) noexcept {
  return window_name + " icon.png";
}

}  // namespace

namespace wb::kernel {

[[nodiscard]] sdl::result<MainWindow> MainWindow::New(
    const char* title, int width, int height,
    sdl::WindowFlags window_flags) noexcept {
  using namespace sdl;

  auto sdl_window =
      sdl::Window::New(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                       width, height, window_flags);
  if (auto* window = sdl::get_result(sdl_window)) WB_ATTRIBUTE_LIKELY {
      const std::string window_icon_name{MakeWindowIconName(title)};

      const auto window_icon_result =
          Surface::FromImage(window_icon_name.c_str());
      if (const auto* window_icon = get_result(window_icon_result))
        WB_ATTRIBUTE_LIKELY { window->SetIcon(*window_icon); }
      else {
        const auto* error = get_error(window_icon_result);
        G3LOG(WARNING) << "SDL unable to set window icon to "
                       << window_icon_name
                       << ", run with default one: " << *error << ".";
      }

      using namespace wb::build::settings::ui::window;

      window->SetMinimumSizes(dimensions::kMinWidth, dimensions::kMinHeight);

      return MainWindow{std::move(*window)};
    }

  return *sdl::get_error(sdl_window);
}

}  // namespace wb::kernel
