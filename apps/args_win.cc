// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Parses wchar_t command line to {argc, argv} tuple on Windows.

#include "args_win.h"

#include <filesystem>
#include <memory>

#include "base/deps/g3log/g3log.h"
#include "base/std2/filesystem_ext.h"
#include "base/std2/string_view_ext.h"
#include "build/compiler_config.h"
//
#include "base/win/windows_light.h"
// Depends on base/win/windows_light.h
#include <shellapi.h>  // CommandLineToArgvW

namespace {

/**
 * @brief Converts wchar_t -> UTF-8.
 * @param wide wchar_t source.
 * @param out_size Out UTF-8 size.
 * @return UTF-8 string.
 */
[[nodiscard]] wb::base::std2::result<wb::base::un<char[]>> WideToUtf8(
    _In_z_ const wchar_t* wide, size_t& out_size) noexcept {
  using namespace wb::base;

  out_size = 0;

  // Compute the size of the required buffer.
  const int size{::WideCharToMultiByte(CP_UTF8, 0, wide, -1, nullptr, 0,
                                       nullptr, nullptr)};
  if (size <= 0) [[unlikely]] {
    // This should never happen.
    const auto rc = std2::system_last_error_code();
    G3PLOG_E(WARNING, rc) << "Could not find required size for command "
                             "line arguments as utf8.";
    return rc;
  }

  // Do the actual conversion.
  un<char[]> argv{std::make_unique<char[]>(static_cast<size_t>(size))};
  if (!argv) [[unlikely]] {
    const auto rc = std2::system_last_error_code(ERROR_OUTOFMEMORY);
    G3PLOG_E(WARNING, rc) << "Could not allocate memory to convert wchar_t "
                             "{argc, argv} to UTF-8.";
    return rc;
  }

  const int result{::WideCharToMultiByte(CP_UTF8, 0, wide, -1, argv.get(), size,
                                         nullptr, nullptr)};
  if (result <= 0) [[unlikely]] {
    // This should never happen.
    const auto rc = std2::system_last_error_code();
    G3PLOG_E(WARNING, rc)
        << "Could not convert command line arguments to utf8.";
    return rc;
  }

  out_size = static_cast<size_t>(size);

  return std::move(argv);
}

/**
 * @brief Wide args wrapper.
 */
struct WideArgs {
  /**
   * @brief Construct wide args from command line.
   * @param command_line Command line.
   */
  explicit WideArgs(_In_z_ const wchar_t* command_line) noexcept
      : argv{::CommandLineToArgvW(command_line, &argc)} {}

  ~WideArgs() noexcept {
    G3PCHECK_E(::LocalFree(argv) == nullptr,
               wb::base::std2::system_last_error_code())
        << "Failed to LocalFree memory from CommandLineToArgvW.";
  }

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(WideArgs);

  /**
   * @brief Argv.
   */
  wchar_t** argv;
  /**
   * @brief Argc.
   */
  int argc;

  [[maybe_unused]] std::byte pad_[sizeof(char*) - sizeof(argc)];
};

}  // namespace

namespace wb::apps::win {

/**
 * @brief Parse command line and build command line arguments pack.
 * @param command_line Command line.
 * @return Parsed command line arguments.
 */
[[nodiscard]] base::std2::result<Args> Args::FromCommandLine(
    _In_z_ const wchar_t* command_line) noexcept {
  using namespace wb::base;

  // Parse command line to wide {argc, argv}.
  WideArgs wargs{command_line};
  wchar_t** wargv{wargs.argv};
  const int argc{wargs.argc};

  if (!wargv || argc <= 0) [[unlikely]] {
    const auto rc = std2::system_last_error_code();
    G3PLOG_E(WARNING, rc)
        << "Could not parse command line to {argc, argv} tuple.";
    return rc;
  }

  // Convert argv to UTF-8.
  un<char*[]> argv{std::make_unique<char*[]>(static_cast<size_t>(argc) + 1)};
  if (!argv) [[unlikely]] {
    const auto rc = std2::system_last_error_code(ERROR_OUTOFMEMORY);
    G3PLOG_E(WARNING, rc) << "Could not allocate memory to convert wchar_t "
                             "{argc, argv} to UTF-8.";
    return rc;
  }

  size_t argv0_size{0};

  for (size_t i{0}; i < static_cast<size_t>(argc); i++) {
    size_t out_size{0};

    auto utf8_result = WideToUtf8(wargv[i], out_size);
    if (auto* arg = std2::get_result(utf8_result)) [[likely]] {
      argv[i] = arg->release();
    } else {
      // Can leak argv[0..i-1], but we stop the app anyway so OS reclaims
      // memory.
      return *std2::get_error(utf8_result);
    }

    if (i == 0) [[unlikely]] {
      argv0_size = out_size;
    }
  }

  char* argv0{argv[0]};
  const auto maybe_short_argv0{
      std2::filesystem::get_short_exe_name_from_command_line(argv0)};
  if (maybe_short_argv0.has_value()) [[likely]] {
    // Abseil expects argv0 is short exe name.
    const std::string short_argv0{maybe_short_argv0.value()};
    strcpy_s(argv0, argv0_size, short_argv0.c_str());
  }

  argv[static_cast<size_t>(argc)] = nullptr;

  return Args{argv.release(), argv0, argc};
}

/**
 * @brief Construct args.
 * @param values Args values.
 * @param count Args count.
 */
Args::Args(_In_ char** values, _In_z_ const char* argv0,
           _In_ int count) noexcept
    : values_{values}, argv0_{argv0}, count_{count} {
  G3DCHECK(!!values);
  G3DCHECK(!!argv0);
  G3DCHECK(count > 0);
}

}  // namespace wb::apps::win
