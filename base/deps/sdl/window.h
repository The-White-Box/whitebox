// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// SDL window wrapper.

#ifndef WB_BASE_DEPS_SDL_WINDOW_H_
#define WB_BASE_DEPS_SDL_WINDOW_H_

#include "base/deps/sdl/base.h"
#include "base/deps/sdl/sdl.h"
#include "base/deps/sdl/surface.h"
#include "base/deps/sdl/syswm.h"
#include "base/deps/sdl/version.h"
#include "base/macroses.h"
#include "base/std2/cstring_ext.h"

namespace wb::sdl {

/**
 * @brief SDL window flags.
 */
enum class WindowFlags : Uint32 {
  kNone = 0U,

  kFullscreen = SDL_WINDOW_FULLSCREEN, /**< fullscreen window */
  kFullscreenDesktop = SDL_WINDOW_FULLSCREEN_DESKTOP,

  kBorderless = SDL_WINDOW_BORDERLESS, /**< no window decoration */
  kResizable = SDL_WINDOW_RESIZABLE,   /**< window can be resized */
  kMinimized = SDL_WINDOW_MINIMIZED,   /**< window is minimized */
  kMaximized = SDL_WINDOW_MAXIMIZED,   /**< window is maximized */

  kUseOpengl = SDL_WINDOW_OPENGL, /**< window usable with OpenGL context */
  kUseVulkan = SDL_WINDOW_VULKAN, /**< window usable for Vulkan surface */
  kUseMetal = SDL_WINDOW_METAL,   /**< window usable for Metal view */

  kShown = SDL_WINDOW_SHOWN,   /**< window is visible */
  kHidden = SDL_WINDOW_HIDDEN, /**< window is not visible */

  kAlwaysOnTop =
      SDL_WINDOW_ALWAYS_ON_TOP, /**< window should always be above others */
  kSkipTaskbar =
      SDL_WINDOW_SKIP_TASKBAR, /**< window should not be added to the taskbar */

  kUtilityWindow =
      SDL_WINDOW_UTILITY, /**< window should be treated as a utility window */
  kTooltip = SDL_WINDOW_TOOLTIP, /**< window should be treated as a tooltip */
  kPopupMenu =
      SDL_WINDOW_POPUP_MENU, /**< window should be treated as a popup menu */

  kInputGrabbed = SDL_WINDOW_INPUT_GRABBED, /**< equivalent to
                        SDL_WINDOW_MOUSE_GRABBED for compatibility */
  kInputHasFocus = SDL_WINDOW_INPUT_FOCUS,  /**< window has input focus */

  kMouseGrabbed =
      SDL_WINDOW_MOUSE_GRABBED, /**< window has grabbed mouse input */
  kMouseHasFocus = SDL_WINDOW_MOUSE_FOCUS, /**< window has mouse focus */
  kMouseHasCapture =
      SDL_WINDOW_MOUSE_CAPTURE, /**< window has mouse captured (unrelated to
                                   MOUSE_GRABBED) */
  kKeyboardGrabbed =
      SDL_WINDOW_KEYBOARD_GRABBED, /**< window has grabbed keyboard input */

  kExternalWindow = SDL_WINDOW_FOREIGN, /**< window not created by SDL */
  kAllowHighDpi =
      SDL_WINDOW_ALLOW_HIGHDPI, /**< window should be created in high-DPI mode
       if supported.  On macOS NSHighResolutionCapable must be set true in the
       application's Info.plist for this to have any effect. */
};

/**
 * @brief operator | for WindowFlags.
 * @param left WindowFlags.
 * @param right WindowFlags.
 * @return left | right.
 */
[[nodiscard]] constexpr WindowFlags operator|(WindowFlags left,
                                              WindowFlags right) noexcept {
  return static_cast<WindowFlags>(base::underlying_cast(left) |
                                  base::underlying_cast(right));
}

/**
 * @brief operator & for WindowFlags.
 * @param left WindowFlags.
 * @param right WindowFlags.
 * @return left & right.
 */
[[nodiscard]] constexpr WindowFlags operator&(WindowFlags left,
                                              WindowFlags right) noexcept {
  return static_cast<WindowFlags>(base::underlying_cast(left) &
                                  base::underlying_cast(right));
}

/**
 * SDL window.
 */
class Window {
 public:
  /**
   * @brief Creates SDL window.
   * @param title Title.
   * @param x X position.
   * @param y Y position.
   * @param width Width.
   * @param height Height.
   * @param flags WindowFlags.
   * @return SDL window.
   */
  static result<Window> New(const char *title, int x, int y, int width,
                            int height, WindowFlags flags) noexcept {
    Window window{::SDL_CreateWindow(title, x, y, width, height,
                                     base::underlying_cast(flags)),
                  flags};
    G3DCHECK(!!window.window_)
        << "SDL_CreateWindow failed with error: " << error::Failure();
    return window.window_ ? result<Window>{std::move(window)}
                          : result<Window>{error::Failure()};
  }
  Window(Window &&w) noexcept : window_{w.window_}, flags_{w.flags_} {
    w.window_ = nullptr;
  }
  Window &operator=(Window &&w) noexcept {
    std::swap(window_, w.window_);
    std::swap(flags_, w.flags_);
    return *this;
  }

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(Window);

  ~Window() noexcept {
    if (window_) {
      ::SDL_DestroyWindow(window_);
      window_ = nullptr;
    }
  }

  /**
   * @brief Get platform-specific window manager information structure.
   * @param platform_info Platform-specific window information.
   * @return Error if any.
   */
  [[nodiscard]] error GetPlatformInfo(
      ::SDL_SysWMinfo &platform_info) const noexcept {
    G3DCHECK(!!window_);

    base::std2::BitwiseMemset(platform_info, 0);
    platform_info.version = GetLinkTimeVersion();

    return error::FromReturnBool(
        ::SDL_GetWindowWMInfo(window_, &platform_info));
  }

  /**
   * @brief Set icon for window.
   * @param icon New icon.
   * @return void.
   */
  void SetIcon(const Surface &icon) const noexcept {
    G3DCHECK(!!window_);

    ::SDL_SetWindowIcon(window_, icon.surface_);
  }

  /**
   * @brief Set minimum window sizes.
   * @param min_width Minimum window width.
   * @param min_height Minimum window height.
   * @return void.
   */
  void SetMinimumSizes(int min_width, int min_height) const noexcept {
    G3DCHECK(!!window_);

    ::SDL_SetWindowMinimumSize(window_, min_width, min_height);
  }

 private:
  /**
   * @brief SDL window.
   */
  SDL_Window *window_;
  /**
   * @brief SDL window flags.
   */
  WindowFlags flags_;

  WB_ATTRIBUTE_UNUSED_FIELD
  std::array<std::byte, sizeof(char *) - sizeof(flags_)> pad_;

  /**
   * Create SDL window.
   * @param window SDL window.
   * @param flags WindowFlags.
   */
  Window(SDL_Window *window, WindowFlags flags) noexcept
      : window_{window}, flags_{flags} {}
};

}  // namespace wb::sdl

#endif  // !WB_BASE_DEPS_SDL_WINDOW_H_
