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
#include "base/intl/lookup.h"
#include "build/build_config.h"

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
             const base::intl::LookupWithFallback &intl_) noexcept
      : app_description{app_description_},
        instance{instance_},
        show_window_flags{show_window_flags_},
        main_icon_id{main_icon_id_},
        small_icon_id{small_icon_id_},
        intl{intl_} {
    G3DCHECK(!!app_description_);
    G3DCHECK(!!instance_);
  }
#else
  KernelArgs(const char *app_description_, char **argv_, const int argc_,
             const base::intl::LookupWithFallback &intl_) noexcept
      : app_description{app_description_},
        argv{argv_},
        argc{argc_},
        intl{intl_} {
    G3DCHECK(!!app_description_);
    G3DCHECK(!!argc_);
    G3DCHECK(!!argv_);
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
#else
  /**
   * App arguments.
   */
  char **argv;
  /**
   * App arguments count.
   */
  const int argc;
#endif

  std::byte pad[4];

  /**
   * @brief Localization service.
   */
  const base::intl::LookupWithFallback &intl;
};
}  // namespace wb::kernel

extern "C" [[nodiscard]] WB_WHITEBOX_KERNEL_API int KernelMain(
    const wb::kernel::KernelArgs &kernel_args);

#endif  // !WB_WHITEBOX_KERNEL_MAIN_H_
