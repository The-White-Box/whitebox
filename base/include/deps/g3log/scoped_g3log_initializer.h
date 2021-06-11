// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Scoped g3log initializer.

#ifndef WB_BASE_INCLUDE_DEPS_G3LOG_SCOPED_G3LOG_INITIALIZER_H_
#define WB_BASE_INCLUDE_DEPS_G3LOG_SCOPED_G3LOG_INITIALIZER_H_
#ifdef _WIN32
#pragma once
#endif

#include <optional>
#include <string>
#include <string_view>

#include "base/include/base_macroses.h"
#include "build/include/build_config.h"
#include "g3log_config.h"

#if defined(WB_OS_POSIX)
#include <signal.h>
#elif defined(WB_OS_WIN)
#include <intrin.h>  // __debugbreak
#endif

WB_COMPILER_MSVC_BEGIN_GLOG_WARNING_OVERRIDE_SCOPE()
#include "g3log.h"
#include "logworker.h"
WB_COMPILER_MSVC_END_GLOG_WARNING_OVERRIDE_SCOPE()

namespace wb::base::deps::g3log {
/**
 * @brief Scoped g3log library initializer.
 */
class ScopedG3LogInitializer {
 public:
  /**
   * @brief Initializes g3log.
   * @param log_prefix Log file name prefix.  May be command line.
   * @param path_to_log_file Path to log file.
   */
  ScopedG3LogInitializer(const std::string_view& log_prefix,
                         const std::string& path_to_log_file)
      : log_worker_{g3::LogWorker::createLogWorker()},
        file_sink_handle_{log_worker_->addDefaultLogger(
            GetExecutableNameFromLogPrefix(log_prefix), path_to_log_file, "")} {
    /** Should be called at very first startup of the software with \ref
     * g3LogWorker pointer. Ownership of the \ref g3LogWorker is the
     * responsibility of the caller */
    g3::initializeLogging(log_worker_.get());
    /** Install signal handler that catches FATAL C-runtime or OS signals
     See the wikipedia site for details http://en.wikipedia.org/wiki/SIGFPE
     See the this site for example usage:
     http://www.tutorialspoint.com/cplusplus/cpp_signal_handling SIGABRT  ABORT
     (ANSI), abnormal termination SIGFPE   Floating point exception (ANSI)
     SIGILL   ILlegal instruction (ANSI)
     SIGSEGV  Segmentation violation i.e. illegal memory reference
     SIGTERM  TERMINATION (ANSI)  */
    g3::installCrashHandler();
    /** setFatalPreLoggingHook() provides an optional extra step before the
     * fatalExitHandler is called
     *
     * Set a function-hook before a fatal message will be sent to the logger
     * i.e. this is a great place to put a break point, either in your debugger
     * or programmatically to catch LOG(FATAL), CHECK(...) or an OS fatal event
     * (exception or signal) This will be reset to default (does nothing) at
     * initializeLogging(...);
     *
     * Example usage:
     * Windows: g3::setFatalPreLoggingHook([]{__debugbreak();}); // remember
     * #include <intrin.h> WARNING: '__debugbreak()' when not running in Debug
     * in your Visual Studio IDE will likely trigger a recursive crash if used
     * here. It should only be used when debugging in your Visual Studio IDE.
     * Recursive crashes are handled but are unnecessary.
     *
     * Linux:   g3::setFatalPreLoggingHook([]{ raise(SIGTRAP); });
     */
#if defined(_DEBUG)
#if defined(WB_OS_POSIX)
    g3::setFatalPreLoggingHook([] { raise(SIGTRAP); });
#elif defined(WB_OS_WIN)
    g3::setFatalPreLoggingHook([] { __debugbreak(); });
#endif
#endif
  }

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedG3LogInitializer);

  ~ScopedG3LogInitializer() {
    // If the LogWorker is initialized then at scope exit the
    // g3::shutDownLogging() will be called.  This is important since it
    // protects from LOG calls from static or other entities that will go out of
    // scope at a later time.
    //
    // It can also be called manually:
    g3::internal::shutDownLogging();
  }

 private:
  std::unique_ptr<g3::LogWorker> log_worker_;
  std::unique_ptr<g3::FileSinkHandle> file_sink_handle_;

  /**
   * @brief Extracts full executable name from log prefix.
   * @param log_prefix Log prefix.
   * @return Full (with extension) executable name.
   */
  static std::optional<std::string_view> GetFullExecutableName(
      std::string_view log_prefix) noexcept {
    // Assume \"x:\zzzzz\yyyy.exe\" www on Windows.
    if (log_prefix.starts_with('"')) {
      const size_t end_exe_double_quote_idx{log_prefix.find('"', 1U)};
      const size_t backslash_before_name_idx{
          log_prefix.rfind('\\', end_exe_double_quote_idx)};

      if (end_exe_double_quote_idx != std::string_view::npos &&
          backslash_before_name_idx != std::string_view::npos) {
        return log_prefix.substr(
            backslash_before_name_idx + 1,
            end_exe_double_quote_idx - backslash_before_name_idx - 1);
      }
    }
    return {};
  }

  /**
   * @brief Trim .exe from executable name.
   * @param exe_name Executable name.
   * @return Executable name without .exe extension.
   */
  static std::string_view TrimExeExtension(std::string_view exe_name) noexcept {
    constexpr char exe_extension[]{".exe"};
    return exe_name.ends_with(exe_extension)
               ? exe_name.substr(0,
                                 exe_name.size() - std::size(exe_extension) + 1)
               : exe_name;
  }

  /**
   * @brief Extracts executable name from log prefix.
   * @param log_prefix Log prefix.
   * @return Executable name.
   */
  static std::string GetExecutableNameFromLogPrefix(
      std::string_view log_prefix) noexcept {
#ifdef WB_OS_POSIX
    const size_t lasts_slash_idx{log_prefix.rfind('/')};
    return std::string{lasts_slash_idx != std::string_view::npos
                           ? log_prefix.substr(lasts_slash_idx + 1)
                           : log_prefix};
#else
    const auto maybe_exe_name = GetFullExecutableName(log_prefix);
    return std::string{maybe_exe_name.has_value()
                           ? TrimExeExtension(maybe_exe_name.value())
                           : log_prefix};
#endif
  }
};
}  // namespace wb::base::deps::g3log

#endif  // WB_BASE_INCLUDE_DEPS_G3LOG_SCOPED_G3LOG_INITIALIZER_H_
