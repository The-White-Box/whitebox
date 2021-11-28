// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Counting stream buffer.

#ifndef WB_BASE_STD2_COUNTING_STREAMBUF_H_
#define WB_BASE_STD2_COUNTING_STREAMBUF_H_

#include <cassert>
#include <sstream>

#include "base/base_macroses.h"

namespace wb::base::std2 {

/**
 * @brief Counting stream buffer.
 */
class countingstreambuf : public std::streambuf {
 public:
  explicit countingstreambuf(std::streambuf* buffer) noexcept
      : buffer_{buffer}, size_{} {
    // Can't use g3log as it may be not initialized yet.
    assert(buffer);
  }

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(countingstreambuf);

  int_type overflow(int c) override {
    if (traits_type::eof() != c) ++size_;

    assert(c <= implicit_cast<int>(std::numeric_limits<char>::max()));

    return buffer_->sputc(static_cast<char>(c));
  }

  int sync() override { return buffer_->pubsync(); }

  /**
   * @brief Gets count of chars in stream.
   * @return Count of chars in stream.
   */
  [[nodiscard]] std::streamsize count() const noexcept { return size_; }

 private:
  std::streambuf* buffer_;
  std::streamsize size_;
};

}  // namespace wb::base::std2

#endif  // !WB_BASE_STD2_COUNTING_STREAMBUF_H_
