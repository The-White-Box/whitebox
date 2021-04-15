// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Half-Life 2 executable command line flags.

#ifndef WB_APPS_HALF_LIFE_2_HL2_EXE_FLAGS_H_
#define WB_APPS_HALF_LIFE_2_HL2_EXE_FLAGS_H_

#include "app_version_config.h"  // WB_PRODUCT_FILE_DESCRIPTION_STRING
#include "apps/base_flags.h"

namespace wb::apps::half_life_2 {

/**
 * @brief Usage message.
 */
constexpr char kUsageMessage[] = WB_PRODUCT_FILE_DESCRIPTION_STRING
    ".  The player again picks up the crowbar of research\nscientist Gordon "
    "Freeman, who finds himself on an alien-infested Earth being\npicked to "
    "the bone, its resources depleted, its populace dwindling.  Freeman is\n"
    "thrust into the unenviable role of rescuing the world from the wrong "
    "he\nunleashed back at Black Mesa.  And a lot of people he cares about are "
    "counting\non him.\n\nSample usage:\n";

}  // namespace wb::apps::half_life_2

#endif  // !WB_APPS_HALF_LIFE_2_HL2_EXE_FLAGS_H_
