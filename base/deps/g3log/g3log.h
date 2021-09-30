// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Collection of g3log logging utilities.

#ifndef WB_BASE_DEPS_G3LOG_G3LOG_H_
#define WB_BASE_DEPS_G3LOG_G3LOG_H_

#include "g3log_config.h"

WB_BEGIN_G3LOG_WARNING_OVERRIDE_SCOPE()
#include "deps/g3log/src/g3log/g3log.hpp"
WB_END_G3LOG_WARNING_OVERRIDE_SCOPE()

#include <system_error>

#include "base/base_macroses.h"
#include "base/std2/system_error_ext.h"
#include "build/compiler_config.h"

#undef LOG
#undef LOG_IF
#undef CHECK
#undef CHECKF
#undef CHECK_F
#undef LOGF
#undef LOGF_IF

namespace wb::base::deps::g3log {
/**
 * @brief Writes error code to stream on out of scope.
 */
class ScopedEndError {
 public:
  /**
   * @brief Create scoped error writer to stream.
   * @param error_code Error code.  Should be first as eval order matters.
   * @param stream Stream to write to.
   * @return nothing.
   */
  ScopedEndError(std::error_code error_code,
                 std::ostringstream &stream) noexcept
      : error_code_{error_code}, stream_{stream} {}

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedEndError);

  /**
   * @brief Write error to stream.
   */
  ~ScopedEndError() noexcept {
    stream_ << " [" << error_code_.category().name() << ": "
            << error_code_.message() << " (" << error_code_.value() << ")].";
  }

  /**
   * @brief Stream to write to.
   * @return Stream to write to.
   */
  [[nodiscard]] std::ostringstream &stream() noexcept { return stream_; }

 private:
  /**
   * @brief Error code to write to stream.
   */
  const std::error_code error_code_;
  /**
   * @brief Stream to write to.
   */
  std::ostringstream &stream_;
};
}  // namespace wb::base::deps::g3log

// G3LOG(level) is the API for the stream log
#define G3LOG(level)                              \
  if (!g3::logLevel(level)) WB_ATTRIBUTE_LIKELY { \
    }                                             \
  else                                            \
    INTERNAL_LOG_MESSAGE(level).stream()

// G3PLOG_E(level, error_code) is the API for the stream log + system error
// code.
#define G3PLOG_E(level, error_code)                       \
  if (!g3::logLevel(level)) WB_ATTRIBUTE_LIKELY {         \
    }                                                     \
  else                                                    \
    wb::base::deps::g3log::ScopedEndError{                \
        error_code, INTERNAL_LOG_MESSAGE(level).stream()} \
        .stream()

// G3PLOG(level) is the API for the stream log + last system error code.
#define G3PLOG(level) G3PLOG_E(level, std2::system_last_error_code())

// 'Conditional' stream log
#define G3LOG_IF(level, boolean_expression)                  \
  if (!g3::logLevel(level) || false == (boolean_expression)) \
    WB_ATTRIBUTE_LIKELY {}                                   \
  else                                                       \
    INTERNAL_LOG_MESSAGE(level).stream()

// 'Conditional' stream log + system error code.
#define G3PLOGE_IF(level, error_code_ptr_expression)                        \
  if (!g3::logLevel(level) || false == (!!(error_code_ptr_expression)))     \
    WB_ATTRIBUTE_LIKELY {}                                                  \
  else                                                                      \
    wb::base::deps::g3log::ScopedEndError{                                  \
        *(error_code_ptr_expression), INTERNAL_LOG_MESSAGE(level).stream()} \
        .stream()

// 'Design By Contract' stream API. Broken Contracts will exit the application
// by using fatal signal SIGABRT
//  For unit testing, you can override the fatal handling using
//  setFatalExitHandler(...). See tes_io.cpp for examples
#define G3CHECK(boolean_expression)                       \
  if (true == (boolean_expression)) WB_ATTRIBUTE_LIKELY { \
    }                                                     \
  else                                                    \
    INTERNAL_CONTRACT_MESSAGE(#boolean_expression).stream()

// 'Design By Contract' stream API + error code. Broken Contracts will exit the
// application by using fatal signal SIGABRT
//  For unit testing, you can override the fatal handling using
//  setFatalExitHandler(...). See tes_io.cpp for examples
#define G3PCHECK_E(boolean_expression, error_code)                           \
  if (true == (boolean_expression)) WB_ATTRIBUTE_LIKELY {                    \
    }                                                                        \
  else                                                                       \
    wb::base::deps::g3log::ScopedEndError{                                   \
        error_code, INTERNAL_CONTRACT_MESSAGE(#boolean_expression).stream()} \
        .stream()

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
#define G3LOGF(level, printf_like_message, ...)   \
  if (!g3::logLevel(level)) WB_ATTRIBUTE_LIKELY { \
    }                                             \
  else                                            \
    INTERNAL_LOG_MESSAGE(level).capturef(printf_like_message, ##__VA_ARGS__)

// Conditional log printf syntax
#define G3LOGF_IF(level, boolean_expression, printf_like_message, ...) \
  if (!g3::logLevel(level) || false == (boolean_expression))           \
    WB_ATTRIBUTE_LIKELY {}                                             \
  else                                                                 \
    INTERNAL_LOG_MESSAGE(level).capturef(printf_like_message, ##__VA_ARGS__)

// Design By Contract, printf-like API syntax with variadic input parameters.
// Calls the signal handler if the contract failed with the default exit for a
// failed contract. This is typically SIGABRT See g3log,
// setFatalExitHandler(...) which can be overriden for unit tests (ref
// test_io.cpp)
#define G3CHECKF(boolean_expression, printf_like_message, ...) \
  if (true == (boolean_expression)) WB_ATTRIBUTE_LIKELY {      \
    }                                                          \
  else                                                         \
    INTERNAL_CONTRACT_MESSAGE(#boolean_expression)             \
        .capturef(printf_like_message, ##__VA_ARGS__)

// Backwards compatible. The same as CHECKF.
// Design By Contract, printf-like API syntax with variadic input parameters.
// Calls the signal handler if the contract failed. See g3log,
// setFatalExitHandler(...) which can be overriden for unit tests (ref
// test_io.cpp)
#define G3CHECK_F(boolean_expression, printf_like_message, ...) \
  if (true == (boolean_expression)) WB_ATTRIBUTE_LIKELY {       \
    }                                                           \
  else                                                          \
    INTERNAL_CONTRACT_MESSAGE(#boolean_expression)              \
        .capturef(printf_like_message, ##__VA_ARGS__)

// Define DEBUG level logging.
#ifdef NDEBUG
// Does nothing.
#define G3DLOG(level)   \
  if constexpr (true) { \
  } else                \
    G3LOG(level)
// Does nothing.
#define G3DPLOG_E(level, error_code) \
  if constexpr (true) {              \
  } else                             \
    G3PLOG(level, error_code)
// Does nothing.
#define G3DPLOG(level)  \
  if constexpr (true) { \
  } else                \
    G3PLOG(level)
// Does nothing.
#define G3DLOG_IF(level, boolean_expression) \
  if constexpr (true) {                      \
  } else                                     \
    G3LOG_IF(level, boolean_expression)
// Does nothing.
#define G3DPLOGE_IF                                         \
  (level, error_code_ptr_expression) if constexpr (true) {} \
  else G3PLOGE_IF(level, error_code_ptr_expression)
// Does nothing.
#define G3DLOGF(level, printf_like_message, ...) \
  if constexpr (true) {                          \
  } else                                         \
    INTERNAL_LOG_MESSAGE(level).capturef(printf_like_message, ##__VA_ARGS__)
// Does nothing.
#define G3DLOGF_IF(level, boolean_expression, printf_like_message, ...) \
  if constexpr (true) {                                                 \
  } else                                                                \
    INTERNAL_LOG_MESSAGE(level).capturef(printf_like_message, ##__VA_ARGS__)
// Does nothing.
#define G3DCHECK(boolean_expression) \
  if constexpr (true) {              \
  } else                             \
    G3CHECK(boolean_expression)
// Does nothing.
#define G3DPCHECK_E(boolean_expression, error_code) \
  if constexpr (true) {                             \
  } else                                            \
    G3PCHECK_E(boolean_expression, error_code)
// Does nothing.
#define G3DCHECKF(boolean_expression, printf_like_message, ...) \
  if constexpr (true) {                                         \
  } else                                                        \
    INTERNAL_CONTRACT_MESSAGE(#boolean_expression)              \
        .capturef(printf_like_message, ##__VA_ARGS__)
#else
// G3DLOG(level) is the API for the stream log in DEBUG mode.
#define G3DLOG(level) G3LOG(level)

// G3DPLOG(level) is the API for the stream log + last system error code in
// DEBUG mode.
#define G3DPLOG(level) G3PLOG(level)

// G3DPLOG_E(level) is the API for the stream log + system error code in
// DEBUG mode.
#define G3DPLOG_E(level, error_code) G3PLOG_E(level, error_code)

// 'Conditional' stream log
#define G3DLOG_IF(level, boolean_expression) G3LOG_IF(level, boolean_expression)

// 'Conditional' stream log + system error code.
#define G3DPLOGE_IF(level, error_code_ptr_expression) \
  G3PLOGE_IF(level, error_code_ptr_expression)

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
   G3DLOGF(INFO, "Characters: %c %c \n", 'a', 65);
   G3DLOGF(INFO, "Decimals: %d %ld\n", 1977, 650000L);      // printing long
   G3DLOGF(INFO, "Preceding with blanks: %10d \n", 1977);
   G3DLOGF(INFO, "Preceding with zeros: %010d \n", 1977);
   G3DLOGF(INFO, "Some different radixes: %d %x %o %#x %#o \n", 100, 100, 100,
100, 100); LOGF(INFO, "floats: %4.2f %+.0e %E \n", 3.1416, 3.1416, 3.1416);
   G3DLOGF(INFO, "Width trick: %*d \n", 5, 10);
   G3DLOGF(INFO, "%s \n", "A string");
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
#define G3DLOGF(level, printf_like_message, ...)  \
  if (!g3::logLevel(level)) WB_ATTRIBUTE_LIKELY { \
    }                                             \
  else                                            \
    INTERNAL_LOG_MESSAGE(level).capturef(printf_like_message, ##__VA_ARGS__)

// Conditional log printf syntax
#define G3DLOGF_IF(level, boolean_expression, printf_like_message, ...) \
  if (false == (boolean_expression) || !g3::logLevel(level))            \
    WB_ATTRIBUTE_LIKELY {}                                              \
  else                                                                  \
    INTERNAL_LOG_MESSAGE(level).capturef(printf_like_message, ##__VA_ARGS__)

// 'Design By Contract' stream API. For Broken Contracts:
//         unit testing: it will throw std::runtime_error when a contract breaks
//         I.R.L : it will exit the application by using fatal signal SIGABRT
#define G3DCHECK(boolean_expression) G3CHECK(boolean_expression)

// 'Design By Contract' stream API + error code. Broken Contracts will exit the
// application by using fatal signal SIGABRT
//  For unit testing, you can override the fatal handling using
//  setFatalExitHandler(...). See tes_io.cpp for examples
#define G3DPCHECK_E(boolean_expression, error_code) \
  G3PCHECK_E(boolean_expression, error_code)

// Design By Contract, printf-like API syntax with variadic input parameters.
// Throws std::runtime_eror if contract breaks
#define G3DCHECKF(boolean_expression, printf_like_message, ...) \
  if (true == (boolean_expression)) WB_ATTRIBUTE_LIKELY {       \
    }                                                           \
  else                                                          \
    INTERNAL_CONTRACT_MESSAGE(#boolean_expression)              \
        .capturef(printf_like_message, ##__VA_ARGS__)
#endif

#endif  // !WB_BASE_DEPS_G3LOG_G3LOG_H_
