// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Parses command line & setup version and usage.

#include "parse_command_line.h"

#include <filesystem>

#include "base/deps/abseil/flags/parse.h"
#include "base/deps/abseil/flags/usage.h"
#include "base/deps/abseil/flags/usage_config.h"
#include "base/deps/abseil/strings/str_cat.h"
#include "base/deps/g3log/g3log.h"

namespace {

/**
 * @brief Makes program version string.
 * @param program_path Full path to program.
 * @param version Program version.
 * @return version string.
 */
[[nodiscard]] std::string VersionString(
    const std::filesystem::path& program_path, std::string_view version) {
  return absl::StrCat(program_path.filename().string(), " version ", version
#ifndef NDEBUG
                      ,
                      " (Debug Build)\n"
#endif
  );
}

}  // namespace

namespace wb::apps {

/**
 * @brief Parses command line flags.
 * @param argc App arguments count.
 * @param argv App arguments.
 * @param parser_config Parser config.
 * @return Not parsed positional flags.
 */
[[nodiscard]] std::vector<char*> ParseCommandLine(
    int argc, char** argv,
    const CommandLineParserConfig& parser_config) noexcept {
  const absl::FlagsUsageConfig flags_usage_config = {
      .contains_helpshort_flags = {},
      .contains_help_flags = {},
      .contains_helppackage_flags = {},
      .version_string =
          [path = argv[0], app_version = parser_config.app_version] {
            return VersionString(std::filesystem::path{path}, app_version);
          },
      .normalize_filename = {}};
  // Set custom version message as we need more info.
  absl::SetFlagsUsageConfig(flags_usage_config);
  // Command line flags should be early initialized, but after logging (depends
  // on it).
  absl::SetProgramUsageMessage(absl::StrCat(parser_config.app_usage, argv[0]));
  std::vector<char*> positional_flags{absl::ParseCommandLine(argc, argv)};

  std::string command_line;
  // Try to optimize for common case.
  command_line.reserve(128U);
  for (int i{0}; i < argc; ++i) {
    absl::StrAppend(&command_line, argv[i]);
    if (i != argc - 1) {
      absl::StrAppend(&command_line, " ");
    }
  }

  G3LOG(INFO) << parser_config.app_name << " started as \"" << command_line
              << "\".";

  return positional_flags;
}

}  // namespace wb::apps
