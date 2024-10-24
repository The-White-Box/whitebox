// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Whitebox kernel main entry point.

#include <chrono>
#include <thread>

#include "main.h"
//
#include "base/deps/fmt/core.h"
#include "base/deps/g3log/g3log.h"
#include "base/deps/sdl/cursor.h"
#include "base/deps/sdl/init.h"
#include "base/deps/sdl/version.h"
#include "base/deps/sdl/window.h"
#include "base/deps/sdl_image/init.h"
#include "base/intl/l18n.h"
#include "build/static_settings_config.h"
#include "kernel/main_window_posix.h"
#include "ui/fatal_dialog.h"

namespace {

/**
 * @brief Run app message loop.
 * @return App exit code.
 */
[[nodiscard]] int DispatchMessages() noexcept {
  SDL_Event event;
  bool is_done{false};

  while (!is_done) {
    while (::SDL_PollEvent(&event) == 1) {
      switch (event.type) {
        case SDL_EVENT_QUIT:
          is_done = true;
          break;

        default:
          continue;
      }
    }

    // TODO(dimhotepus): Do smth when no events.
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(5ms);
  }

  return 0;
}

/**
 * @brief Make window flags.
 * @return Window flags.
 */
[[nodiscard]] constexpr wb::sdl::WindowFlags MakeWindowFlags() noexcept {
  using namespace wb::sdl;

  // TODO(dimhotepus): kAllowHighDpi handling at least on Mac.
  // Hidden by default, as we check either it is second instance or not and show
  // only if not.
  return WindowFlags::kResizable | WindowFlags::kHidden |
         WindowFlags::kAllowHighDpi
#if defined(WB_OS_LINUX) || defined(WB_OS_WIN)
         | WindowFlags::kUseVulkan
#elif defined(WB_OS_MACOS)
         | WindowFlags::kUseMetal
#else
#error "Please, define SDL window flags for your platform."
#endif
      ;
}

/**
 * @brief Get used by window graphics context.
 * @param flags WindowFlags.
 * @return Window graphics context.
 */
[[nodiscard]] constexpr const char* GetWindowGraphicsContext(
    wb::sdl::WindowFlags flags) noexcept {
  if ((flags & wb::sdl::WindowFlags::kUseOpengl) ==
      wb::sdl::WindowFlags::kUseOpengl) {
    return "OpenGL";
  }

  if ((flags & wb::sdl::WindowFlags::kUseVulkan) ==
      wb::sdl::WindowFlags::kUseVulkan) {
    return "Vulkan";
  }

  if ((flags & wb::sdl::WindowFlags::kUseMetal) ==
      wb::sdl::WindowFlags::kUseMetal) {
    return "Metal";
  }

  return "N/A";
}

/**
 * @brief Makes fatal dialog context.
 * @param kernel_args Kernel arguments.
 * @return Fatal dialog context.
 */
[[nodiscard]] wb::ui::FatalDialogContext MakeFatalContext(
    const wb::kernel::KernelArgs& kernel_args) noexcept {
#ifdef WB_OS_WIN
  return wb::ui::FatalDialogContext{kernel_args.intl, kernel_args.intl.Layout(),
                                    kernel_args.main_icon_id,
                                    kernel_args.small_icon_id};
#else
  return wb::ui::FatalDialogContext{kernel_args.intl.Layout()};
#endif
}

}  // namespace

extern "C" [[nodiscard]] WB_WHITEBOX_KERNEL_API int KernelMain(
    const wb::kernel::KernelArgs& kernel_args) {
  using namespace wb::base;
  using namespace wb::kernel;

  const auto& intl = kernel_args.intl;
  const auto& command_line_flags = kernel_args.command_line_flags;

  using namespace wb::sdl;
  using namespace wb::sdl_image;

  const int compiled_sdl_version{GetCompileTimeVersion()},
      linked_sdl_version{GetLinkTimeVersion()};
  const auto sdl_initializer = SDLInitializer::New(SDLInitializerFlags::kAudio |
                                                   SDLInitializerFlags::kVideo);
  if (const auto* error = get_error(sdl_initializer)) [[unlikely]] {
    return wb::ui::FatalDialog(
        intl::l18n_fmt(intl, "{0} - Error", kernel_args.app_description), {},
        intl::l18n(intl,
                   "Please, check your SDL library installed and working."),
        MakeFatalContext(kernel_args),
        intl::l18n_fmt(
            intl,
            "SDL build/runtime v.{0}.{1}.{2}/v.{3}.{4}.{5}, revision '{6}' "
            "initialization failed.\n\n{7}.",
            SDL_VERSIONNUM_MAJOR(compiled_sdl_version),
            SDL_VERSIONNUM_MINOR(compiled_sdl_version),
            SDL_VERSIONNUM_MICRO(compiled_sdl_version),
            SDL_VERSIONNUM_MAJOR(linked_sdl_version),
            SDL_VERSIONNUM_MINOR(linked_sdl_version),
            SDL_VERSIONNUM_MICRO(linked_sdl_version), ::SDL_GetRevision(),
            *error));
  }

  // Try to use wait cursor while window is created.  Should go after SDL init.
  un<ScopedCursor> wait_cursor_while_app_starts{
      //-V821
      CreateScopedCursor(SystemCursor::kWaitArrow)};

  G3LOG(INFO) << "SDL versions: build " << compiled_sdl_version << ", runtime "
              << linked_sdl_version << '.';

  constexpr SDLImageInitType sdl_image_init_types{SDLImageInitType::kJpg |
                                                  SDLImageInitType::kPng};

  const auto sdl_image_init = SDLImageInit::New(sdl_image_init_types);
  if (const auto* error = get_error(sdl_image_init)) [[unlikely]] {
    return wb::ui::FatalDialog(
        intl::l18n_fmt(intl, "{0} - Error", kernel_args.app_description), {},
        intl::l18n(intl,
                   "Please, check your SDL library installed and working."),
        MakeFatalContext(kernel_args),
        intl::l18n_fmt(intl,
                       "SDL image parser initialization failed for image "
                       "types {0}.\n\n{1}.",
                       sdl_image_init_types, *error));
  }

  const WindowFlags window_flags{MakeWindowFlags()};

  const auto window_result = MainWindow::New(
      kernel_args.app_description, command_line_flags.main_window_width,
      command_line_flags.main_window_height, window_flags, intl);
  if ([[maybe_unused]] const auto* window = get_result(window_result)) [[likely]] {
    G3LOG(INFO) << "SDL graphics context: "
                << GetWindowGraphicsContext(window_flags) << ".";

    {
      G3LOG(INFO) << "SDL window subsystem: Posix.";
    }

    // Startup sequence finished, window is already shown, restore default
    // cursor.
    wait_cursor_while_app_starts.reset();

    return DispatchMessages();
  }

  const auto* error = get_error(window_result);
  G3DCHECK(!!error);

  return wb::ui::FatalDialog(
      intl::l18n_fmt(intl, "{0} - Error", kernel_args.app_description),
      error->code,
      intl::l18n_fmt(intl,
                     "Please, check you installed '{0}' libraries/drivers.",
                     GetWindowGraphicsContext(window_flags)),
      MakeFatalContext(kernel_args),
      intl::l18n_fmt(intl,
                     "SDL window create failed with '{0}' context.\n\n{1}",
                     GetWindowGraphicsContext(window_flags), *error));
}