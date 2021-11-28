// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Whitebox kernel main entry point.

#ifndef WB_WHITEBOX_KERNEL_MAIN_H_
#define WB_WHITEBOX_KERNEL_MAIN_H_

#include <cstddef>  // std::byte

#include "api.h"
#include "base/deps/g3log/g3log.h"
#include "base/intl/lookup_with_fallback.h"
#include "build/build_config.h"
#include "whitebox-boot-manager/command_line_flags.h"

#ifdef WB_OS_WIN
/**
 * @brief HINSTANCE type.
 */
using HINSTANCE = struct HINSTANCE__ *;
#endif

namespace wb::kernel {

/**
 * @brief Kernel args.
 */
struct KernelArgs {
#ifdef WB_OS_WIN
  KernelArgs(const char *app_description_, HINSTANCE instance_,
             int show_window_flags_, int main_icon_id_, int small_icon_id_,
             const wb::boot_manager::CommandLineFlags &command_line_flags_,
             const base::intl::LookupWithFallback &intl_) noexcept
      : app_description{app_description_},
        instance{instance_},
        show_window_flags{show_window_flags_},
        main_icon_id{main_icon_id_},
        small_icon_id{small_icon_id_},
        command_line_flags{command_line_flags_},
        intl{intl_} {
    G3DCHECK(!!app_description_);
    G3DCHECK(!!instance_);
  }
#else
  KernelArgs(const char *app_description_,
             const wb::boot_manager::CommandLineFlags &command_line_flags_,
             const base::intl::LookupWithFallback &intl_) noexcept
      : app_description{app_description_},
        command_line_flags{command_line_flags_},
        intl{intl_} {
    G3DCHECK(!!app_description_);
  }
#endif

  /**
   * @brief App description.
   */
  const char *app_description;

#ifdef WB_OS_WIN
  /**
   * @brief App instance.
   */
  HINSTANCE instance;
  /**
   * @brief Show app window flags.
   */
  int show_window_flags;
  /**
   * @brief Main app icon id.
   */
  int main_icon_id;
  /**
   * @brief Small app icon id.
   */
  int small_icon_id;
#endif

  /**
   * @brief Command line flags.
   */
  const boot_manager::CommandLineFlags &command_line_flags;

  /**
   * @brief Localization service.
   */
  const base::intl::LookupWithFallback &intl;

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(KernelArgs);
};

}  // namespace wb::kernel

extern "C" [[nodiscard]] WB_WHITEBOX_KERNEL_API int KernelMain(
    const wb::kernel::KernelArgs &kernel_args);

#endif  // !WB_WHITEBOX_KERNEL_MAIN_H_
