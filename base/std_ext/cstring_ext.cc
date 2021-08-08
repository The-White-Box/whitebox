// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// <cstring> extensions.

#include "cstring_ext.h"

#include <cstdlib>  // *_s

#include "base/deps/g3log/g3log.h"
#include "base/std_ext/system_error_ext.h"

namespace wb::base::std_ext {
#ifdef WB_OS_WIN
[[nodiscard]] WB_BASE_API std::string WideToAnsi(const std::wstring &in) {
  const std::size_t in_size{in.size() + 1};
  std::string ansi(in_size, '\0');

  [[maybe_unused]] const std::error_code rc{std_ext::GetThreadPosixErrorCode(
      ::wcstombs_s(nullptr, &ansi[0], in_size, in.data(), in_size))};
  // Only debug as invalid CRT parameter handler expected to terminate app.
  G3DCHECK(!rc);

  ansi.resize(in_size - 1);

  return ansi;
}

[[nodiscard]] WB_BASE_API std::wstring AnsiToWide(const std::string &in) {
  const std::size_t in_size{in.size() + 1};
  std::wstring wide(in_size, '\0');

  [[maybe_unused]] const std::error_code rc{std_ext::GetThreadPosixErrorCode(
      ::mbstowcs_s(nullptr, &wide[0], in_size, in.data(), in_size))};
  // Only debug as invalid CRT parameter handler expected to terminate app.
  G3DCHECK(!rc);

  wide.resize(in_size - 1);

  return wide;
}
#endif
}  // namespace wb::base::std_ext
