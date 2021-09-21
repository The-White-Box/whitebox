// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// SDL window wrapper.

#ifndef WB_BASE_DEPS_SDL_WINDOW_H_
#define WB_BASE_DEPS_SDL_WINDOW_H_

#include "base/base_macroses.h"
#include "base/deps/sdl/base.h"
#include "base/deps/sdl/sdl.h"
#include "base/deps/sdl/surface.h"
#include "base/deps/sdl/syswm.h"
#include "base/deps/sdl/version.h"
#include "base/std2/cstring_ext.h"

namespace wb::sdl {
/**
 * @brief SDL window flags.
 */
enum class SdlWindowFlags : Uint32 {
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
 * @brief operator | for SdlWindowFlags.
 * @param left SdlWindowFlags.
 * @param right SdlWindowFlags.
 * @return left | right.
 */
[[nodiscard]] constexpr SdlWindowFlags operator|(
    SdlWindowFlags left, SdlWindowFlags right) noexcept {
  return static_cast<SdlWindowFlags>(base::underlying_cast(left) |
                                     base::underlying_cast(right));
}

/**
 * @brief operator & for SdlWindowFlags.
 * @param left SdlWindowFlags.
 * @param right SdlWindowFlags.
 * @return left & right.
 */
[[nodiscard]] constexpr SdlWindowFlags operator&(
    SdlWindowFlags left, SdlWindowFlags right) noexcept {
  return static_cast<SdlWindowFlags>(base::underlying_cast(left) &
                                     base::underlying_cast(right));
}

/**
 * SDL window.
 */
class SdlWindow {
 public:
  /**
   * @brief Creates SDL window.
   * @param title Title.
   * @param x X position.
   * @param y Y position.
   * @param width Width.
   * @param height Height.
   * @param flags SdlWindowFlags.
   * @return SDL window.
   */
  static SdlResult<SdlWindow> New(const char *title, int x, int y, int width,
                                  int height, SdlWindowFlags flags) noexcept {
    SdlWindow window{title, x, y, width, height, flags};
    return window.error_code().IsSucceeded()
               ? SdlResult<SdlWindow>{std::move(window)}
               : SdlResult<SdlWindow>{window.error_code()};
  }
  SdlWindow(SdlWindow &&w) noexcept
      : window_{w.window_}, init_rc_{w.init_rc_}, flags_{w.flags_} {
    w.window_ = nullptr;
    w.init_rc_ = SdlError::Success();
  }
  SdlWindow &operator=(SdlWindow &&w) noexcept {
    std::swap(window_, w.window_);
    std::swap(init_rc_, w.init_rc_);
    std::swap(flags_, w.flags_);
    return *this;
  }

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(SdlWindow);

  ~SdlWindow() noexcept {
    if (window_) {
      ::SDL_DestroyWindow(window_);
      window_ = nullptr;
    }
  }

  [[nodiscard]] SdlError GetPlatformInfo(
      ::SDL_SysWMinfo &platform_info) const noexcept {
    G3DCHECK(!!window_);

    base::std2::BitwiseMemset(platform_info, 0);
    platform_info.version = GetLinkTimeVersion();

    return SdlError::FromReturnBool(
        ::SDL_GetWindowWMInfo(window_, &platform_info));
  }

  void SetIcon(const SdlSurface &icon) const noexcept {
    G3DCHECK(!!window_);

    ::SDL_SetWindowIcon(window_, icon.surface_);
  }

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
   * @brief SDL window initialization code.
   */
  SdlError init_rc_;
  /**
   * @brief SDL window flags.
   */
  SdlWindowFlags flags_;
  WB_ATTRIBUTE_UNUSED_FIELD std::byte pad_[sizeof(char *) - sizeof(flags_)];

  /**
   * Creates SDL window.
   * @param title Title.
   * @param x X position.
   * @param y Y position.
   * @param width Window width.
   * @param height Window height.
   * @param flags SdlWindowFlags.
   */
  SdlWindow(const char *title, int x, int y, int width, int height,
            SdlWindowFlags flags) noexcept
      : window_{::SDL_CreateWindow(title, x, y, width, height,
                                   base::underlying_cast(flags))},
        init_rc_{window_ ? SdlError::Success() : SdlError::Failure()},
        flags_{flags} {
    G3DCHECK(!!window_) << "SDL_CreateWindow failed with error: "
                        << error_code();
  }

  /**
   * @brief Init result.
   * @return SDL error.
   */
  [[nodiscard]] SdlError error_code() const noexcept { return init_rc_; }
};
}  // namespace wb::sdl

#endif  // !WB_BASE_DEPS_SDL_WINDOW_H_
