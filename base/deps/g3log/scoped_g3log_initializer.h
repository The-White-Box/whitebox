// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Scoped g3log initializer.

#ifndef WB_BASE_DEPS_G3LOG_SCOPED_G3LOG_INITIALIZER_H_
#define WB_BASE_DEPS_G3LOG_SCOPED_G3LOG_INITIALIZER_H_

#include <cassert>
#include <iostream>
#include <string>
#include <string_view>

#include "base/base_macroses.h"
#include "base/std2/filesystem_ext.h"
#include "base/std2/counting_streambuf.h"
#include "base/std2/string_view_ext.h"
#include "build/build_config.h"
#include "g3log_config.h"

#if defined(WB_OS_POSIX)
#include <csignal>
#elif defined(WB_OS_WIN)
#include <intrin.h>  // __debugbreak
#endif

#include "console_sink.h"
#include "g3log.h"
#include "logworker.h"

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
            GetExecutableNameFromLogPrefix(log_prefix), path_to_log_file, "")},
        console_sink_handle_{log_worker_->addSink(
            std::make_unique<ConsoleSink>(), &ConsoleSink::ReceiveLogMessage)},
        g3_initializer_{log_worker_.get()},
        g3_io_streams_redirector_{} {
    // Install signal handler that catches FATAL C-runtime or OS signals
    // See the wikipedia site for details http://en.wikipedia.org/wiki/SIGFPE
    // See this site for example usage:
    // http://www.tutorialspoint.com/cplusplus/cpp_signal_handling
    // SIGABRT  ABORT (ANSI), abnormal termination
    // SIGFPE   Floating point exception (ANSI)
    // SIGILL   Illegal instruction (ANSI)
    // SIGSEGV  Segmentation violation i.e. illegal memory reference
    // SIGTERM  TERMINATION (ANSI)
    g3::installCrashHandler();
    // setFatalPreLoggingHook() provides an optional extra step before the
    // fatalExitHandler is called
    //
    // Set a function-hook before a fatal message will be sent to the logger
    // i.e. this is a great place to put a break point, either in your debugger
    // or programmatically to catch LOG(FATAL), CHECK(...) or an OS fatal event
    // (exception or signal) This will be reset to default (does nothing) at
    // initializeLogging(...);
    //
    // Example usage:
    // Windows: g3::setFatalPreLoggingHook([]{__debugbreak();});
    // Remember #include <intrin.h>
    // WARNING: '__debugbreak()' when not running in Debug in your Visual Studio
    // IDE will likely trigger a recursive crash if used here.  It should only
    // be used when debugging in your Visual Studio IDE.  Recursive crashes are
    // handled but are unnecessary.
    //
    // Linux:   g3::setFatalPreLoggingHook([]{ raise(SIGTRAP); });
#if !defined(NDEBUG)
#if defined(WB_OS_POSIX)
    g3::setFatalPreLoggingHook([] { raise(SIGTRAP); });
#elif defined(WB_OS_WIN)
    g3::setFatalPreLoggingHook([] { __debugbreak(); });
#endif
#endif
  }

  WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(ScopedG3LogInitializer);

  /**
   * @brief Shut down g3log.
   */
  ~ScopedG3LogInitializer() noexcept = default;

 private:
  /**
   * @brief g3log initializer.
   */
  struct G3LogInitializer {
    explicit G3LogInitializer(g3::LogWorker* log_worker) noexcept {
      /** Should be called at very first startup of the software with \ref
       * g3LogWorker pointer. Ownership of the \ref g3LogWorker is the
       * responsibility of the caller */
      g3::initializeLogging(log_worker);
    }

    WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(G3LogInitializer);

    ~G3LogInitializer() noexcept {
      // If the LogWorker is initialized then at scope exit the
      // g3::shutDownLogging() will be called.  This is important since it
      // protects from LOG calls from static or other entities that will go out
      // of scope at a later time.
      g3::internal::shutDownLogging();
    }
  };

  /**
   * @brief g3log cout / cerr redirector.
   */
  struct G3IoStreamsRedirector {
    G3IoStreamsRedirector()
        : cout_{std::ios_base::out},
          cerr_{std::ios_base::out},
          cout_stream_buf_{cout_.rdbuf()},
          cerr_stream_buf_{cerr_.rdbuf()},
          old_cout_stream_buf_{std::cout.rdbuf(&cout_stream_buf_)},
          old_cerr_stream_buf_{std::cerr.rdbuf(&cerr_stream_buf_)} {}

    WB_NO_COPY_MOVE_CTOR_AND_ASSIGNMENT(G3IoStreamsRedirector);

    ~G3IoStreamsRedirector() noexcept {
      std::cerr.rdbuf(old_cerr_stream_buf_);
      std::cout.rdbuf(old_cout_stream_buf_);

      if (cerr_stream_buf_.count() != 0) {
        // cerr will write to WARNING.
        INTERNAL_LOG_MESSAGE(WARNING).stream().swap(cerr_);
      }

      if (cout_stream_buf_.count() != 0) {
        // cout will write to INFO.
        INTERNAL_LOG_MESSAGE(INFO).stream().swap(cout_);
      }
    }

    std::ostringstream cout_;
    std::ostringstream cerr_;

    std2::countingstreambuf cout_stream_buf_;
    std2::countingstreambuf cerr_stream_buf_;

    std::streambuf* old_cout_stream_buf_;
    std::streambuf* old_cerr_stream_buf_;
  };

  /**
   * @brief Log worker.
   */
  wb::base::un<g3::LogWorker> log_worker_;
  /**
   * @brief File sink handle.
   */
  wb::base::un<g3::FileSinkHandle> file_sink_handle_;
  /**
   * @brief Console sink handle.
   */
  wb::base::un<g3::SinkHandle<ConsoleSink>> console_sink_handle_;
  /**
   * @brief g3log initializer.
   */
  G3LogInitializer g3_initializer_;

  WB_ATTRIBUTE_UNUSED_FIELD std::byte pad_[7];

  /**
   * @brief g3log cout / cerr redirector.  Depends on g3_initializer_.
   */
  G3IoStreamsRedirector g3_io_streams_redirector_;

  /**
   * @brief Trim .exe from executable name.
   * @param exe_name Executable name.
   * @return Executable name without .exe extension.
   */
  static std::string_view TrimExeExtension(std::string_view exe_name) noexcept {
    constexpr char exe_extension[]{".exe"};
    return std2::ends_with(exe_name, exe_extension)
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
    const auto maybe_exe_name =
        std2::filesystem::get_short_exe_name_from_command_line(log_prefix);
    return std::string{maybe_exe_name.has_value()
                           ? TrimExeExtension(maybe_exe_name.value())
                           : log_prefix};
#endif
  }
};

}  // namespace wb::base::deps::g3log

#endif  // WB_BASE_DEPS_G3LOG_SCOPED_G3LOG_INITIALIZER_H_
