// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Static UI settings configuration.

#ifndef WB_UI_STATIC_SETTINGS_CONFIG_H_
#define WB_UI_STATIC_SETTINGS_CONFIG_H_

#include "build/build_config.h"

namespace wb::ui::settings {

namespace window::dimensions {
// https://www.microsoft.com/en-us/windows/windows-10-specifications
// On Windows 10 minimum display dimensions are 800x600, so do not use more
// than.
constexpr int kMinWidth{640};
constexpr int kMinHeight{480};
}  // namespace window::dimensions

}  // namespace wb::ui::settings

#endif  // !WB_UI_STATIC_SETTINGS_CONFIG_H_
