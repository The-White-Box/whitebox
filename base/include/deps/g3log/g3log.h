// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Collection of g3log logging utilities.

#ifndef WB_BASE_INCLUDE_DEPS_G3LOG_G3LOG_H_
#define WB_BASE_INCLUDE_DEPS_G3LOG_G3LOG_H_
#ifdef _WIN32
#pragma once
#endif

#include "g3log_config.h"

WB_COMPILER_MSVC_BEGIN_GLOG_WARNING_OVERRIDE_SCOPE()
#include "deps/g3log/src/g3log/g3log.hpp"
WB_COMPILER_MSVC_END_GLOG_WARNING_OVERRIDE_SCOPE()

// Define DEBUG level logging.
#ifdef NDEBUG
// Does nothing.
#define DLOG(level)     \
  if constexpr (true) { \
  } else                \
    LOG(level)
// Does nothing.
#define DLOG_IF(level, boolean_expression) \
  if constexpr (true) {                    \
  } else                                   \
    LOG_IF(level, boolean_expression)
// Does nothing.
#define DLOGF(level, printf_like_message, ...) \
  if constexpr (true) {                        \
  } else                                       \
    INTERNAL_LOG_MESSAGE(level).capturef(printf_like_message, ##__VA_ARGS__)
// Does nothing.
#define DLOGF_IF(level, boolean_expression, printf_like_message, ...) \
  if constexpr (true) {                                               \
  } else                                                              \
    INTERNAL_LOG_MESSAGE(level).capturef(printf_like_message, ##__VA_ARGS__)
// Does nothing.
#define DCHECK(boolean_expression) \
  if constexpr (true) {            \
  } else                           \
    CHECK(boolean_expression)
// Does nothing.
#define DCHECKF(boolean_expression, printf_like_message, ...) \
  if constexpr (true) {                                       \
  } else                                                      \
    INTERNAL_CONTRACT_MESSAGE(#boolean_expression)           \
        .capturef(printf_like_message, ##__VA_ARGS__)
#else
// LOG(level) is the API for the stream log in DEBUG mode.
#define DLOG(level) LOG(level)
// 'Conditional' stream log
#define DLOG_IF(level, boolean_expression) LOG_IF(level, boolean_expression)
/** For details please see this
 * REFERENCE: http://www.cppreference.com/wiki/io/c/printf_format
 * \verbatim
 *
  There are different %-codes for different variable types, as well as options
to limit the length of the variables and whatnot. Code Format
    %[flags][width][.precision][length]specifier
 SPECIFIERS
 ----------
 %c character
 %d signed integers
 %i signed integers
 %e scientific notation, with a lowercase “e”
 %E scientific notation, with a uppercase “E”
 %f floating point
 %g use %e or %f, whichever is shorter
 %G use %E or %f, whichever is shorter
 %o octal
 %s a string of characters
 %u unsigned integer
 %x unsigned hexadecimal, with lowercase letters
 %X unsigned hexadecimal, with uppercase letters
 %p a pointer
 %n the argument shall be a pointer to an integer into which is placed the
number of characters written so far

For flags, width, precision etc please see the above references.
EXAMPLES:
{
   LOGF(INFO, "Characters: %c %c \n", 'a', 65);
   LOGF(INFO, "Decimals: %d %ld\n", 1977, 650000L);      // printing long
   LOGF(INFO, "Preceding with blanks: %10d \n", 1977);
   LOGF(INFO, "Preceding with zeros: %010d \n", 1977);
   LOGF(INFO, "Some different radixes: %d %x %o %#x %#o \n", 100, 100, 100, 100,
100); LOGF(INFO, "floats: %4.2f %+.0e %E \n", 3.1416, 3.1416, 3.1416);
   LOGF(INFO, "Width trick: %*d \n", 5, 10);
   LOGF(INFO, "%s \n", "A string");
   return 0;
}
And here is possible output
:      Characters: a A
:      Decimals: 1977 650000
:      Preceding with blanks:       1977
:      Preceding with zeros: 0000001977
:      Some different radixes: 100 64 144 0x64 0144
:      floats: 3.14 +3e+000 3.141600E+000
:      Width trick:    10
:      A string  \endverbatim */
#define DLOGF(level, printf_like_message, ...) \
  if (!g3::logLevel(level)) {                  \
  } else                                       \
    INTERNAL_LOG_MESSAGE(level).capturef(printf_like_message, ##__VA_ARGS__)
// Conditional log printf syntax
#define DLOGF_IF(level, boolean_expression, printf_like_message, ...) \
  if (false == (boolean_expression) || !g3::logLevel(level)) {        \
  } else                                                              \
    INTERNAL_LOG_MESSAGE(level).capturef(printf_like_message, ##__VA_ARGS__)
// 'Design By Contract' stream API. For Broken Contracts:
//         unit testing: it will throw std::runtime_error when a contract breaks
//         I.R.L : it will exit the application by using fatal signal SIGABRT
#define DCHECK(boolean_expression) CHECK(boolean_expression)
// Design By Contract, printf-like API syntax with variadic input parameters.
// Throws std::runtime_eror if contract breaks
#define DCHECKF(boolean_expression, printf_like_message, ...) \
  if (true == (boolean_expression)) {                         \
  } else                                                      \
    INTERNAL_CONTRACT_MESSAGE(#boolean_expression)            \
        .capturef(printf_like_message, ##__VA_ARGS__)
#endif

#endif  // !WB_BASE_INCLUDE_DEPS_G3LOG_G3LOG_H_
