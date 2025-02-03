// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// <string_view> extensions.

#include "string_view_ext.h"

#include <cstring>  // strncpy | strncpy_s

#include "base/deps/g3log/g3log.h"

namespace wb::base::std2 {

/**
 * @brief Trim spaces around data.
 * @param in Input to trim spaces around.
 * @param out Output to write |in| with trimmed spaces to.
 * @param out_size Output buffer size.
 * @return true if smth trimmed, false otherwise.
 */
WB_BASE_API bool TrimSpaces(std::string_view in, char *out,
                            std::size_t out_size) noexcept {
  if (!in.size() && out_size > 0) {
    out[0] = '\0';
    return true;
  }

  if (!out) return false;

  size_t i{0};
  // Trim leading space.
  while (i < in.size() && std::isspace(in[i])) ++i;

  if (i == in.size()) {
    if (out_size > 0) {
      out[0] = '\0';
      return true;
    }

    return false;
  }

  // Trim trailing space.
  const char *end{in.data() + in.size() - 1}, *end_copy{end};

  while (end > in.data() && std::isspace(*end)) end--;

  const size_t size{static_cast<size_t>(end + 1 - in.data()) + i};
  // Overflow.
  if (size >= out_size) return false;

#ifdef WB_OS_WIN
  strncpy_s(out, out_size, in.data() + i, size);
#else
  strncpy(out, in.data() + i, size);
  out[size] = '\0';
#endif  // WB_OS_WIN

  return i > 0 || end != end_copy;
}

}  // namespace wb::base::std2