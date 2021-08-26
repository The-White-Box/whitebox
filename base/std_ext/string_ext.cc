// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// <string> extensions.

#include "string_ext.h"

#include "base/deps/g3log/g3log.h"

#ifdef WB_OS_WIN
#include "base/windows/windows_light.h"
#include "build/compiler_config.h"
//
#include <WinNls.h>
#endif

namespace wb::base::std_ext {
#ifdef WB_OS_WIN
[[nodiscard]] WB_BASE_API std::string WideToUTF8(const std::wstring &source) {
  if (source.empty() ||
      source.size() > static_cast<size_t>(std::numeric_limits<int>::max())) {
    return std::string{};
  }
  const int size{::WideCharToMultiByte(CP_UTF8, 0, &source[0],
                                       static_cast<int>(source.size()), nullptr,
                                       0, nullptr, nullptr)};
  G3DCHECK(size > 0);
  std::string result(static_cast<size_t>(size), '\0');
  if (::WideCharToMultiByte(CP_UTF8, 0, &source[0],
                            static_cast<int>(source.size()), &result[0], size,
                            nullptr, nullptr) != size) {
    WB_COMPILER_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()
      // conditional expression is constant
      WB_COMPILER_MSVC_DISABLE_WARNING(4127)
      G3DCHECK(false);
    WB_COMPILER_MSVC_END_WARNING_OVERRIDE_SCOPE()
    return std::string{};
  }
  return result;
}

[[nodiscard]] WB_BASE_API std::wstring UTF8ToWide(const std::string &source) {
  if (source.empty() ||
      source.size() > static_cast<size_t>(std::numeric_limits<int>::max())) {
    return std::wstring{};
  }
  const int size{::MultiByteToWideChar(
      CP_UTF8, 0, &source[0], static_cast<int>(source.size()), nullptr, 0)};
  G3DCHECK(size > 0);
  std::wstring result(static_cast<size_t>(size), L'\0');
  if (::MultiByteToWideChar(CP_UTF8, 0, &source[0],
                            static_cast<int>(source.size()), &result[0],
                            size) != size) {
    WB_COMPILER_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()
      // conditional expression is constant
      WB_COMPILER_MSVC_DISABLE_WARNING(4127)
      G3DCHECK(false);
    WB_COMPILER_MSVC_END_WARNING_OVERRIDE_SCOPE()
    return std::wstring{};
  }
  return result;
}
#endif
}  // namespace wb::base::std_ext
