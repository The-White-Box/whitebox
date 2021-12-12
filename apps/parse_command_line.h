// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Parses command line & setup version and usage.

#ifndef WB_APPS_PARSE_COMMAND_LINE_
#define WB_APPS_PARSE_COMMAND_LINE_

#include <string_view>
#include <vector>

namespace wb::apps {

/**
 * @brief Command line parser configuration.
 */
struct CommandLineParserConfig {
  /**
   * @brief Application name,
   */
  std::string_view app_name;
  /**
   * @brief Application version.
   */
  std::string_view app_version;
  /**
   * @brief Application usage.
   */
  std::string_view app_usage;
};

/**
 * @brief Parses command line flags.
 * @param argc App arguments count.
 * @param argv App arguments.
 * @param parser_config Parser config.
 * @return Not parsed positional flags.
 */
[[nodiscard]] std::vector<char*> ParseCommandLine(
    int argc, char** argv,
    const CommandLineParserConfig& parser_config) noexcept;

}  // namespace wb::apps

#endif  // WB_APPS_PARSE_COMMAND_LINE_
