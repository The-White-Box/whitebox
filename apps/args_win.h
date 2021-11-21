// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Parses wchar_t command line to {argc, argv} tuple on Windows.

#ifndef WB_APPS_ARGS_WIN_H_
#define WB_APPS_ARGS_WIN_H_

#include <sal.h>

#include <cstddef>  // std::byte

#include "base/base_macroses.h"
#include "base/std2/system_error_ext.h"

namespace wb::apps::win {

/**
 * @brief Command line arguments.
 */
class Args {
 public:
  /**
   * @brief Parse command line and build command line arguments pack.
   * @param command_line Command line.
   * @return Parsed command line arguments.
   */
  [[nodiscard]] static wb::base::std2::result<Args> FromCommandLine(
      _In_z_ const wchar_t* command_line) noexcept;

  Args(Args&& args) noexcept
      : values_{args.values_}, argv0_{args.argv0_}, count_{args.count_} {
    args.values_ = nullptr;
    args.argv0_ = nullptr;
    args.count_ = 0;
  }

  Args& operator=(Args&& args) noexcept = delete;

  ~Args() noexcept {
    for (int i = 0; i < count_; i++) {
      delete[] values_[i];
    }

    delete[] values_;
    count_ = 0;
  }

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(Args);

  /**
   * @brief Args count.
   * @return Args count.
   */
  [[nodiscard]] int count() const noexcept { return count_; }

  /**
   * @brief Args values.
   * @return Args values.
   */
  [[nodiscard]] char** values() const noexcept { return values_; }

  /**
   * @brief Argv 0 value.
   * @return Argv 0 value.
   */
  [[nodiscard]] const char* argv0() const noexcept { return argv0_; }

 private:
  /**
   * @brief Args values.
   */
  char** values_;
  /**
   * @brief Arg 0 value.
   */
  const char* argv0_;
  /**
   * @brief Count of args.
   */
  int count_;

  [[maybe_unused]] std::byte pad_[sizeof(char*) - sizeof(count_)];

  /**
   * @brief Construct args.
   * @param values Args values.
   * @param count Args count.
   */
  Args(_In_ char** values, _In_z_ const char* argv0, _In_ int count) noexcept;
};

}  // namespace wb::apps::win

#endif  // !WB_APPS_ARGS_WIN_H_
