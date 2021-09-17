// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Bootmgr main entry point.

#include "bootmgr_main.h"

#include <filesystem>

#include "app_version_config.h"
#include "base/deps/g3log/g3log.h"
#include "base/deps/mimalloc/mimalloc.h"
#include "base/intl/clocale_ext.h"
#include "base/intl/lookup.h"
#include "base/scoped_floating_point_mode.h"
#include "base/scoped_process_terminate_handler.h"
#include "base/scoped_process_unexpected_handler.h"
#include "base/scoped_shared_library.h"
#include "base/std2/filesystem_ext.h"
#include "base/threads/thread_utils.h"
#include "build/build_config.h"
#include "whitebox-kernel/whitebox_kernel_main.h"
#ifdef WB_OS_WIN
#include "base/win/dll_load_utils.h"
#include "base/win/error_handling/scoped_pure_call_handler.h"
#include "base/win/error_handling/scoped_thread_invalid_parameter_handler.h"
#include "base/win/memory/memory_utils.h"
#include "base/win/memory/scoped_new_handler.h"
#include "base/win/memory/scoped_new_mode.h"
#include "base/win/mmcss/scoped_mmcss_thread_controller.h"
#include "base/win/scoped_minimum_timer_resolution.h"
#include "base/win/security/process_mitigations.h"
#include "base/win/ui/task_dialog.h"
#include "base/win/windows_version.h"
#include "build/static_settings_config.h"
#else
#include "base/scoped_new_handler.h"
#endif

namespace {
/**
 * Dump some system information.
 * @param app_description Application description.
 */
void DumpSystemInformation(const char* app_description) noexcept {
  G3DCHECK(!!app_description);

#if defined(WB_OS_LINUX) || defined(WB_OS_MACOSX)
#if defined(WB_COMPILER_CLANG)
  constexpr char kCompilerVersion[]{"Clang " __clang_version__};
#elif defined(WB_COMPILER_GCC)
  const std::string kCompilerVersion{std::to_string(__GNUC__) + "." +
                                     std::to_string(__GNUC_MINOR__) + "." +
                                     std::to_string(__GNUC_PATCHLEVEL__)};
#else
  constexpr char kCompilerVersion[];
#error Please, add your compiler build version here.
#endif  // WB_COMPILER_GCC

#if defined(WB_LIBC_GLIBC) && defined(_GLIBCXX_RELEASE)
  G3LOG(INFO) << app_description << " v." << WB_PRODUCT_FILEVERSION_INFO_STRING
              << " build with " << kCompilerVersion << " on glibc " << __GLIBC__
              << "." << __GLIBC_MINOR__ << ", glibc++ " << _GLIBCXX_RELEASE
              << ", ABI stamp " << __GLIBCXX__ << ".";
#endif
#ifdef _LIBCPP_VERSION
  G3LOG(INFO) << app_description << " v." << WB_PRODUCT_FILEVERSION_INFO_STRING
              << " build with " << kCompilerVersion << " on libc++ "
              << _LIBCPP_VERSION << "/ ABI " << _LIBCPP_ABI_VERSION;
#endif
#endif  // WB_OS_LINUX || WB_OS_MACOSX

#ifdef WB_OS_WIN
  G3LOG(INFO) << app_description << " v." << WB_PRODUCT_FILEVERSION_INFO_STRING
              << " build with MSVC " << _MSC_FULL_VER << '.' << _MSC_BUILD
              << ".";
#endif
}

/**
 * @brief Creates internationalization lookup.
 * @param user_locale User locale.
 * @return Internationalization lookup.
 */
[[nodiscard]] wb::base::intl::LookupWithFallback CreateIntl(
    const std::string& user_locale) noexcept {
  auto intl_lookup_result{
      wb::base::intl::LookupWithFallback::New({user_locale})};
  auto intl_lookup =
      std::get_if<wb::base::intl::LookupWithFallback>(&intl_lookup_result);

  G3LOG_IF(FATAL, !intl_lookup)
      << "Unable to create localization strings lookup for locale "
      << user_locale << ".";
  return std::move(*intl_lookup);
}

/**
 * @brief Shows fatal error box and exits.
 * @param rc Error code for technical details.
 * @param intl Localization lookup.
 * @param main_instruction_message Main instruction message.
 * @param content_message Content message.
 * @param main_icon_id Main icon id for error box.
 * @param small_icon_id Small icon id for error box.
 * @return
 */
[[noreturn]] void FatalErrorBox(std::error_code rc,
                                wb::base::intl::LookupWithFallback& intl,
                                const std::string& main_instruction_message,
                                const std::string& content_message,
                                int main_icon_id, int small_icon_id) noexcept {
  using namespace wb::base;
  using namespace wb::base::windows;

  const std::string technical_details{rc.message()};
  const bool rtl_layout{intl.Layout() ==
                        intl::Lookup::StringLayout::RightToLeft};

  ui::DialogBoxSettings dialog_settings(
      nullptr, intl.String(intl::message_ids::kBootmgrErrorDialogTitle),
      main_instruction_message, content_message,
      intl.String(intl::message_ids::kHideTechnicalDetails),
      intl.String(intl::message_ids::kSeeTechnicalDetails), technical_details,
      intl.String(intl::message_ids::kNudgeAuthorsLink),
      ui::DialogBoxButton::kOk, main_icon_id, small_icon_id, rtl_layout);
  ui::ShowDialogBox(ui::DialogBoxKind::kError, dialog_settings);

  G3PLOG_E(FATAL, rc) << main_instruction_message;
}

/**
 * @brief Setup heap allocator.
 */
void BootHeapMemoryAllocator() noexcept {
#ifdef WB_OS_WIN
  {
    // Terminate the app if system detected heap corruption.
    const auto error_code =
        wb::base::windows::memory::EnableTerminationOnHeapCorruption();
    G3PLOGE_IF(FATAL, !!error_code, error_code)
        << "Can't enable 'Terminate on Heap corruption' os feature, continue "
           "without it.";
  }

  {
    // Optimize heap caches now.
    const auto error_code =
        wb::base::windows::memory::OptimizeHeapResourcesNow();
    G3PLOGE_IF(WARNING, !!error_code, error_code)
        << "Can't optimize heap resources caches, some memory will not be "
           "freed.";
  }
#endif

#ifdef WB_USE_MI_MALLOC
  // Ignore earlier allocations.
  mi_stats_reset();

  G3DLOG(INFO) << "Using mi-malloc memory allocator v." << mi_version() << ".";
#endif
}

/**
 * @brief Load and run kernel.
 * @param bootmgr_args Boot manager args.
 * @param intl Localization lookup.
 * @return App exit code.
 */
int KernelStartup(const wb::bootmgr::BootmgrArgs& bootmgr_args,
                  wb::base::intl::LookupWithFallback& intl) noexcept {
  using namespace wb::base;

  std::error_code rc;
  auto app_path = std2::GetExecutableDirectory(rc);
  if (rc) [[unlikely]] {
    FatalErrorBox(
        rc, intl,
        intl.String(
            intl::message_ids::kCantGetExecutableDirectoryForBootManager),
        intl.String(intl::message_ids::kPleaseNudgeAuthors),
        bootmgr_args.main_icon_id, bootmgr_args.small_icon_id);
  }

#ifdef WB_OS_WIN
  const std::string kernel_path{(app_path /= "whitebox-kernel.dll").string()};
#else
  app_path /= "libwhitebox-kernel.so." WB_PRODUCT_VERSION_INFO_STRING;

  const std::string kernel_path{app_path.string()};
#endif

#ifdef WB_OS_WIN
  const unsigned kernel_load_flags{
      LOAD_WITH_ALTERED_SEARCH_PATH |
      (windows::MustBeSignedDllLoadTarget(bootmgr_args.command_line)
           ? LOAD_LIBRARY_REQUIRE_SIGNED_TARGET
           : 0U)};
#else
  const int kernel_load_flags{RTLD_LAZY | RTLD_LOCAL};
#endif

  const auto kernel_load_result =
      ScopedSharedLibrary::FromLibraryOnPath(kernel_path, kernel_load_flags);
  if (const auto* kernel_module = std2::GetSuccessResult(kernel_load_result))
      [[likely]] {
    using KernelMainFunction = decltype(&KernelMain);
    constexpr char kKernelMainFunctionName[]{"KernelMain"};

    // Good, try to find and launch whitebox-kernel.
    const auto kernel_main_result =
        kernel_module->GetAddressAs<KernelMainFunction>(
            kKernelMainFunctionName);
    if (const auto* kernel_main = std2::GetSuccessResult(kernel_main_result))
        [[likely]] {
#ifdef WB_OS_WIN
      return (*kernel_main)(
          {bootmgr_args.instance, bootmgr_args.app_description,
           bootmgr_args.show_window_flags, bootmgr_args.main_icon_id,
           bootmgr_args.small_icon_id});
#else
      return (*kernel_main)(
          {bootmgr_args.app_description, bootmgr_args.argv, bootmgr_args.argc});
#endif
    }

    FatalErrorBox(std::get<std::error_code>(kernel_main_result), intl,
                  intl.String(intl::message_ids::kPleaseReinstallTheGame),
                  intl.StringFormat(intl::message_ids::kCantGetKernelEntryPoint,
                                    fmt::make_format_args(
                                        kKernelMainFunctionName, kernel_path)),
                  bootmgr_args.main_icon_id, bootmgr_args.small_icon_id);
  } else {
    FatalErrorBox(std::get<std::error_code>(kernel_load_result), intl,
                  intl.String(intl::message_ids::kPleaseReinstallTheGame),
                  intl.StringFormat(intl::message_ids::kCantLoadKernelFrom,
                                    fmt::make_format_args(kernel_path)),
                  bootmgr_args.main_icon_id, bootmgr_args.small_icon_id);
  }
}
}  // namespace

/**
 * @brief Bootmgr entry point on Windows.
 * @param bootmgr_args Bootmgr args.
 * @return 0 on success.
 */
extern "C" [[nodiscard]] WB_BOOTMGR_API int BootmgrMain(
    const wb::bootmgr::BootmgrArgs& bootmgr_args) {
  using namespace wb::base;

  DumpSystemInformation(bootmgr_args.app_description);

  // Start with specifying UTF-8 locale for all data.
  const intl::ScopedProcessLocale scoped_process_locale{
      intl::ScopedProcessLocaleCategory::kAll, intl::locales::kUtf8Locale};
  const std::string& user_locale{
      scoped_process_locale.GetCurrentLocale().value_or(
          intl::locales::kFallbackLocale)};
  G3LOG(INFO) << bootmgr_args.app_description << " using " << user_locale
              << " locale for UI.";

  auto intl = CreateIntl(user_locale);

#ifdef WB_OS_WIN
  using namespace wb::base::windows;

  // Requires Windows Version 1903+ for UTF-8 process code page.
  //
  // "With a minimum target version of Windows Version 1903, the process code
  // page will always be UTF-8 so legacy code page detection and conversion can
  // be avoided.".
  //
  // See
  // https://docs.microsoft.com/en-us/windows/uwp/design/globalizing/use-utf8-code-page#set-a-process-code-page-to-utf-8
  if (windows::GetVersion() < windows::Version::WIN10_19H1) [[unlikely]] {
    FatalErrorBox(std2::GetThreadErrorCode(ERROR_OLD_WIN_VERSION), intl,
                  intl.String(intl::message_ids::kPleaseUpdateWindowsVersion),
                  intl.String(intl::message_ids::kWindowsVersionIsTooOld),
                  bootmgr_args.main_icon_id, bootmgr_args.small_icon_id);
  }

  // Enable process attacks mitigation policies in scope.
  const auto scoped_process_mitigation_policies =
      security::ScopedProcessMitigationPolicies::New();
  G3PLOGE_IF(WARNING,
             !!std2::GetErrorCode(scoped_process_mitigation_policies),
             *std2::GetErrorCode(scoped_process_mitigation_policies))
      << "Can't enable process attacks mitigation policies, attacker can use "
         "system features to break in app.";

  // Handle call of CRT function with bad arguments on the thread.
  const error_handling::ScopedThreadInvalidParameterHandler
      scoped_thread_invalid_parameter_handler{
          error_handling::DefaultThreadInvalidParameterHandler};
  // Handle pure virtual function call.
  const error_handling::ScopedPureCallHandler scoped_pure_call_handler{
      error_handling::DefaultPureCallHandler};
  // Handle new allocation failure.
  const memory::ScopedNewHandler scoped_new_handler{
      memory::DefaultNewFailureHandler};
  // Call new when malloc failed.
  const memory::ScopedNewMode scoped_new_mode{
      memory::ScopedNewModeFlag::CallNew};
#else
  // Handle new allocation failure.
  const ScopedNewHandler scoped_new_handler{DefaultNewFailureHandler};
#endif

  // Handle terminate function call on the thread.
  const ScopedProcessTerminateHandler scoped_process_terminate_handler{
      DefaultProcessTerminateHandler};
  // Handle unexpected function call on the thread.
  const ScopedProcessUnexpectedHandler scoped_process_unexpected_handler{
      DefaultProcessUnexpectedHandler};

#ifdef WB_OS_WIN
  const threads::NativeThreadName new_thread_name{L"WhiteBoxMain"};
#else
  const threads::NativeThreadName new_thread_name{"WhiteBoxMain"};
#endif

  // Mark main thread with name to simplify debugging.
  const auto scoped_thread_name = threads::ScopedThreadName::New(
      threads::GetCurrentThreadHandle(), new_thread_name);
  G3PLOGE_IF(WARNING, !!std2::GetErrorCode(scoped_thread_name),
             *std2::GetErrorCode(scoped_thread_name))
      << "Can't rename main thread, continue with default name.";

  // Setup heap memory allocator.
  BootHeapMemoryAllocator();

  // Ensure CPU floating point units convert denormals to zero and flush to zero
  // on underflow.
  const ScopedFloatingPointMode scoped_floating_point_mode{
      ScopedFloatingPointModeFlags::kDenormalsAreZero |
      ScopedFloatingPointModeFlags::kFlushToZero};

#ifdef WB_OS_WIN
  // Set minimum timers resolution to good enough, but not too power hungry.
  const auto scoped_minimum_timer_resolution =
      windows::ScopedMinimumTimerResolution::New(std::chrono::milliseconds{
          wb::build::settings::kMinimumTimerResolutionMs});
  G3LOG_IF(WARNING, !!std::get_if<unsigned>(&scoped_minimum_timer_resolution))
      << "Failed to set minimum periodic timers resolution to "
      << wb::build::settings::kMinimumTimerResolutionMs
      << "ms, will run with default system one: ("
      << std::get<unsigned>(scoped_minimum_timer_resolution)
      << "ms).  See "
         "https://docs.microsoft.com/en-us/windows/win32/api/timeapi/"
         "nf-timeapi-timebeginperiod";

  {
    // Signal Multimedia Class Scheduler Service we have game thread here, so
    // utilize as much of the CPU as possible without denying CPU resources to
    // lower-priority applications.
    const windows::mmcss::ScopedMmcssThreadTask game_task{
        windows::mmcss::KnownScopedMmcssThreadTaskName::kGames};
    const windows::mmcss::ScopedMmcssThreadTask playback_task{
        windows::mmcss::KnownScopedMmcssThreadTaskName::kPlayback};
    const auto scoped_mmcss_thread_controller =
        windows::mmcss::ScopedMmcssThreadController::New(game_task,
                                                         playback_task);
    if (const auto* controller = std2::GetSuccessResult(
            scoped_mmcss_thread_controller)) [[likely]] {
      const auto responsiveness_percent =
          controller->GetResponsivenessPercent();

      if (const auto* percent =
              std2::GetSuccessResult(responsiveness_percent)) [[likely]] {
        G3DLOG(INFO) << "Multimedia Class Scheduler Service uses "
                     << implicit_cast<unsigned>(*percent)
                     << "% system responsiveness value.";
      } else {
        G3PLOG_E(WARNING, *std2::GetErrorCode(responsiveness_percent))
            << "Can't get system responsiveness setting used by Multimedia "
               "Class Scheduler Service for the main app thread.";
      }

      const auto bump_thread_priority_rc = controller->SetPriority(
          windows::mmcss::ScopedMmcssThreadPriority::kHigh);
      G3PLOGE_IF(WARNING, !!bump_thread_priority_rc, bump_thread_priority_rc)
          << "Can't get system responsiveness setting used by Multimedia "
             "Class Scheduler Service for the thread.";
    } else {
      G3PLOG_E(WARNING, *std2::GetErrorCode(scoped_mmcss_thread_controller))
          << "Can't enable Multimedia Class Scheduler Service for the app, "
             "some CPU resources may be underutilized.  See "
             "https://docs.microsoft.com/en-us/windows/win32/procthread/"
             "multimedia-class-scheduler-service#registry-settings";
    }
  }
#endif

  return KernelStartup(bootmgr_args, intl);
}