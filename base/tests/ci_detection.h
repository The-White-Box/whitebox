// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Detect various CIs.

#ifndef WB_BASE_TESTS_CI_DETECTION_H_
#define WB_BASE_TESTS_CI_DETECTION_H_

#include <cstdlib>

#include "base/deps/g3log/g3log.h"
#include "base/std2/system_error_ext.h"
#include "build/compiler_config.h"

namespace wb::base::tests_internal {

/**
 * @brief Is code running in CI pipeline?
 * @param is_ci true if CI pipeline detected, false otherwise.
 * @return true if successfully queried CI pipeline presense, false otherwise.
 */
[[nodiscard]] inline bool IsInContinuousIntegrationPipeline(bool &is_ci) noexcept {
  constexpr char kGithubActionsEnvVariableName[] = "GITHUB_ACTIONS";

#ifdef WB_OS_WIN
  // Peak env var size is 32767:
  // https://devblogs.microsoft.com/oldnewthing/20100203-00/?p=15083
  char github_ci_env[SHRT_MAX];
  github_ci_env[0] = '\0';

  size_t count;

  const int rc{getenv_s(&count, github_ci_env, kGithubActionsEnvVariableName)};
  if (!rc) {
    is_ci = github_ci_env[0] != '\0';
    return true;
  }

  G3PCHECK_E(rc == 0, wb::base::std2::posix_last_error_code(rc))
      << "Unable to read env variable " << kGithubActionsEnvVariableName;
  return false;
#else
  const char *github_ci_env{getenv(kGithubActionsEnvVariableName)};
  is_ci = !!github_ci_env;

  return true;
#endif
}

}  // namespace wb::base::tests_internal

#endif  // !WB_BASE_TESTS_CI_DETECTION_H_
