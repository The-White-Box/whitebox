// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// App main window on POSIX.

#include "main_window_posix.h"

#include <string>

#include "base/scoped_app_instance_manager.h"
#include "build/static_settings_config.h"
#include "whitebox-ui/fatal_dialog.h"

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

      using namespace wb::base;

      // Check only single instance of the app is running.  Do it here because
      // on Linux / MacOS we want to show fatal dialog when app has icon, hense
      // postpone check till SDL window created and app icon set.
      const ScopedAppInstanceManager scoped_app_instance_manager{title};
      const auto other_instance_status =
          scoped_app_instance_manager.GetStatus();
      if (other_instance_status == AppInstanceStatus::kAlreadyRunning)
        WB_ATTRIBUTE_UNLIKELY {
          wb::ui::FatalDialog(
              intl::l18n(boot_manager_args.intl, "Boot Manager - Error"),
              std2::posix_last_error_code(EEXIST),
              intl::l18n_fmt(boot_manager_args.intl,
                             "Sorry, only single '{0}' can run at a time.",
                             boot_manager_args.app_description),
              MakeFatalContext(boot_manager_args),
              intl::l18n_fmt(
                  boot_manager_args.intl,
                  "Can't run multiple copies of '{0}' at once.  Please, "
                  "stop existing copy or return to the game.",
                  boot_manager_args.app_description));
        }

      return MainWindow{std::move(*window)};
    }

  return *sdl::get_error(sdl_window);
}

}  // namespace wb::kernel
