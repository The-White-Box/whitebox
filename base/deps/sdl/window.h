// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// SDL window wrapper.

#ifndef WB_BASE_DEPS_SDL_WINDOW_H_
#define WB_BASE_DEPS_SDL_WINDOW_H_

#include <string>
#include <string_view>

#include "base/deps/sdl/base.h"
#include "base/deps/sdl/sdl.h"
#include "base/deps/sdl/surface.h"
#include "base/deps/sdl/version.h"
#include "base/macroses.h"
#include "base/std2/cstring_ext.h"

namespace wb::sdl {

/**
 * @brief SDL window flags.
 */
enum class WindowFlags : SDL_WindowFlags {
  kNone = 0U,

  kFullscreen = SDL_WINDOW_FULLSCREEN, /**< fullscreen window */

  kBorderless = SDL_WINDOW_BORDERLESS, /**< no window decoration */
  kResizable = SDL_WINDOW_RESIZABLE,   /**< window can be resized */
  kMinimized = SDL_WINDOW_MINIMIZED,   /**< window is minimized */
  kMaximized = SDL_WINDOW_MAXIMIZED,   /**< window is maximized */

  kUseOpengl = SDL_WINDOW_OPENGL, /**< window usable with OpenGL context */
  kUseVulkan = SDL_WINDOW_VULKAN, /**< window usable for Vulkan surface */
  kUseMetal = SDL_WINDOW_METAL,   /**< window usable for Metal view */

  kHidden = SDL_WINDOW_HIDDEN, /**< window is not visible */

  kAlwaysOnTop =
      SDL_WINDOW_ALWAYS_ON_TOP, /**< window should always be above others */

  kUtilityWindow =
      SDL_WINDOW_UTILITY, /**< window should be treated as a utility window */
  kTooltip = SDL_WINDOW_TOOLTIP, /**< window should be treated as a tooltip */
  kPopupMenu =
      SDL_WINDOW_POPUP_MENU, /**< window should be treated as a popup menu */

  kInputHasFocus = SDL_WINDOW_INPUT_FOCUS, /**< window has input focus */

  kMouseGrabbed =
      SDL_WINDOW_MOUSE_GRABBED, /**< window has grabbed mouse input */
  kMouseHasFocus = SDL_WINDOW_MOUSE_FOCUS, /**< window has mouse focus */
  kMouseHasCapture =
      SDL_WINDOW_MOUSE_CAPTURE, /**< window has mouse captured (unrelated to
                                   MOUSE_GRABBED) */
  kKeyboardGrabbed =
      SDL_WINDOW_KEYBOARD_GRABBED, /**< window has grabbed keyboard input */

  kAllowHighDpi =
      SDL_WINDOW_HIGH_PIXEL_DENSITY, /**< window should be created in high-DPI
       mode
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
  static result<Window> New(std::string_view title, int x, int y, int width,
                            int height, WindowFlags flags) noexcept {
    const std::string window_title{title};

    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, window_title.c_str());
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, x);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, y);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, width);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, height);
    // For window flags you should use separate window creation properties,
    // but for easier migration from SDL2 you can use the following:
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_FLAGS_NUMBER,
                          static_cast<Sint64>(base::underlying_cast(flags)));
    Window window{::SDL_CreateWindowWithProperties(props), flags};
    SDL_DestroyProperties(props);

    const wb::sdl::error rc{error::Failure()};

    G3DCHECK(!!window.window_)
        << "SDL_CreateWindow failed with error: " << rc;
    return window.window_ ? result<Window>{std::move(window)}
                          : result<Window>{std::unexpect, rc};
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

  /**
   * Toggles window visibility.
   * @param should_show Show window or not.
   */
  void Toggle(bool should_show) const noexcept {
    G3DCHECK(!!window_);

    if (should_show) {
      ::SDL_ShowWindow(window_);
    } else {
      ::SDL_HideWindow(window_);
    }
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
