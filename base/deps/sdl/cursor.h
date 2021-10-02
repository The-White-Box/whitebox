// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// SDL cursor wrapper.

#ifndef WB_BASE_DEPS_SDL_CURSOR_H_
#define WB_BASE_DEPS_SDL_CURSOR_H_

#include <type_traits>

#include "base/deps/sdl/base.h"
#include "base/deps/sdl/mouse.h"
#include "base/deps/sdl/sdl.h"

namespace wb::sdl {

/**
 * @brief SDL system cursor.
 */
enum class SdlSystemCursor : std::underlying_type_t<SDL_SystemCursor> {
  kArrow = SDL_SYSTEM_CURSOR_ARROW,         /**< Arrow */
  kIBeam = SDL_SYSTEM_CURSOR_IBEAM,         /**< I-beam */
  kWait = SDL_SYSTEM_CURSOR_WAIT,           /**< Wait */
  kCrosshair = SDL_SYSTEM_CURSOR_CROSSHAIR, /**< Crosshair */
  kWaitArrow = SDL_SYSTEM_CURSOR_WAITARROW, /**< Small wait cursor (or Wait if
                                               not available) */
  kSizeNWSE = SDL_SYSTEM_CURSOR_SIZENWSE,   /**< Double arrow pointing northwest
                                               and southeast */
  kSizeNESW = SDL_SYSTEM_CURSOR_SIZENESW,   /**< Double arrow pointing northeast
                                               and southwest */
  kSizeWE =
      SDL_SYSTEM_CURSOR_SIZEWE, /**< Double arrow pointing west and east */
  kSizeNS =
      SDL_SYSTEM_CURSOR_SIZENS, /**< Double arrow pointing north and south */
  kSizeAll = SDL_SYSTEM_CURSOR_SIZEALL, /**< Four pointed arrow pointing north,
                                           south, east, and west */
  kNo = SDL_SYSTEM_CURSOR_NO,           /**< Slashed circle or crossbones */
  KHand = SDL_SYSTEM_CURSOR_HAND,       /**< Hand */
  kCursorsCount = SDL_NUM_SYSTEM_CURSORS
};

/**
 * @brief SDL cursor.
 */
class SdlCursor {
 public:
  /**
   * @brief Create system cursor.
   * @param system_cursor SdlSystemCursor.
   * @return
   */
  [[nodiscard]] static SdlResult<SdlCursor> FromSystem(
      SdlSystemCursor system_cursor) noexcept {
    SdlCursor cursor{system_cursor};
    return cursor.error_code().is_succeeded()
               ? SdlResult<SdlCursor>{std::move(cursor)}
               : SdlResult<SdlCursor>{cursor.error_code()};
  }

  /**
   * @brief Create system cursor.
   * @param system_cursor SdlSystemCursor.
   * @return
   */
  [[nodiscard]] static SdlResult<SdlCursor> FromActive() noexcept {
    SdlCursor cursor;
    return cursor.error_code().is_succeeded()
               ? SdlResult<SdlCursor>{std::move(cursor)}
               : SdlResult<SdlCursor>{cursor.error_code()};
  }

  /**
   * @brief Empty cursor placeholder.
   * @return Empty cursor placeholder.
   */
  [[nodiscard]] static SdlCursor Empty() noexcept { return SdlCursor{false}; }

  SdlCursor(SdlCursor &&c) noexcept : cursor_{c.cursor_}, init_rc_{c.init_rc_} {
    c.cursor_ = nullptr;
    c.init_rc_ = SdlError::Failure();
  }
  SdlCursor &operator=(SdlCursor &&c) noexcept {
    std::swap(cursor_, c.cursor_);
    std::swap(init_rc_, c.init_rc_);
    return *this;
  }

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(SdlCursor);

  ~SdlCursor() noexcept {
    if (cursor_) {
      ::SDL_FreeCursor(cursor_);
      cursor_ = nullptr;
    }
  }

  /**
   * @brief Mae cursor current (active).
   */
  void MakeActive() const noexcept {
    G3DCHECK(!!cursor_);
    ::SDL_SetCursor(cursor_);
  }

 private:
  ::SDL_Cursor *cursor_;
  SdlError init_rc_;

  /**
   * @brief Create system SDL cursor.
   * @param system_cursor System cursor.
   */
  explicit SdlCursor(SdlSystemCursor system_cursor) noexcept
      : cursor_{::SDL_CreateSystemCursor(
            base::enum_cast<SDL_SystemCursor>(system_cursor))},
        init_rc_{cursor_ ? SdlError::Success() : SdlError::Failure()} {}

  /**
   * @brief Create active cursor.
   */
  SdlCursor() noexcept
      : cursor_{::SDL_GetCursor()},
        // Active cursor may be nullptr if no mouse.
        init_rc_{SdlError::Success()} {}

  /**
   * @brief Create empty cursor.
   */
  explicit SdlCursor(bool) noexcept
      : cursor_{nullptr}, init_rc_{SdlError::Success()} {}

  /**
   * @brief Error code.
   * @return Error code.
   */
  [[nodiscard]] SdlError error_code() const noexcept { return init_rc_; }
};

/**
 * Changes cursor in scope.
 */
class ScopedSdlCursor {
 public:
  /**
   * @brief Scoped SDL cursor.
   * @param new_cursor Cursor to use in scope.
   */
  explicit ScopedSdlCursor(SdlCursor &&new_cursor) noexcept
      : old_cursor_{SdlCursor::FromActive()},
        new_cursor_{std::move(new_cursor)} {
    new_cursor_.MakeActive();
  }

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(ScopedSdlCursor);

  ScopedSdlCursor(ScopedSdlCursor &&c) noexcept
      : old_cursor_(std::move(c.old_cursor_)),
        new_cursor_{std::move(c.new_cursor_)} {
    c.old_cursor_ = SdlResult<SdlCursor>{SdlError::Failure("Empty")};
    c.new_cursor_ = SdlCursor::Empty();
  }
  ScopedSdlCursor &operator=(ScopedSdlCursor &&c) noexcept = delete;

  ~ScopedSdlCursor() noexcept {
    if (auto *old_cursor = get_result(old_cursor_)) WB_ATTRIBUTE_LIKELY {
        old_cursor->MakeActive();
      }
  }

 private:
  SdlResult<SdlCursor> old_cursor_;
  SdlCursor new_cursor_;
};

/**
 * @brief Tries to change cursor to system one in scope.
 * @param new_cursor_in_scope New system cursor.
 * @return ScopedSdlCursor.
 */
[[nodiscard]] inline wb::base::un<wb::sdl::ScopedSdlCursor> CreateScopedCursor(
    wb::sdl::SdlSystemCursor new_cursor_in_scope) noexcept {
  using namespace wb::sdl;

  auto new_cursor = SdlCursor::Empty();
  auto system_cursor = SdlCursor::FromSystem(new_cursor_in_scope);
  if (auto *cursor = get_result(system_cursor)) WB_ATTRIBUTE_LIKELY {
      new_cursor = std::move(*cursor);
    }
  return std::make_unique<ScopedSdlCursor>(std::move(new_cursor));
}

}  // namespace wb::sdl

#endif  // !WB_BASE_DEPS_SDL_CURSOR_H_
