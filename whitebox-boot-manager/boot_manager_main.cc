// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Boot manager main entry point.

#include "boot_manager_main.h"

#include <filesystem>

#include "app_version_config.h"
#include "base/deps/g3log/g3log.h"
#include "base/deps/mimalloc/mimalloc.h"
#include "base/intl/l18n.h"
#include "base/scoped_floating_point_mode.h"
#include "base/scoped_process_terminate_handler.h"
#include "base/scoped_shared_library.h"
#include "base/std2/filesystem_ext.h"
#include "build/build_config.h"
#include "scoped_app_instance_manager.h"
#include "whitebox-kernel/main.h"
#include "whitebox-ui/fatal_dialog.h"

#ifdef WB_OS_WIN
#include "base/std2/thread_ext.h"
#include "base/win/dll_load_utils.h"
#include "base/win/error_handling/scoped_process_pure_call_handler.h"
#include "base/win/error_handling/scoped_thread_invalid_parameter_handler.h"
#include "base/win/memory/memory_utils.h"
#include "base/win/memory/scoped_new_handler.h"
#include "base/win/memory/scoped_new_mode.h"
#include "base/win/mmcss/scoped_mmcss_thread_controller.h"
#include "base/win/scoped_timer_resolution.h"
#include "base/win/security/process_mitigations.h"
#include "base/win/windows_version.h"
#include "build/static_settings_config.h"
#include "whitebox-kernel/main_window_win.h"
#include "whitebox-ui/win/window_utilities.h"
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

#if defined(WB_OS_POSIX)
#if defined(WB_COMPILER_CLANG)
  // NOLINTNEXTLINE(modernize-avoid-c-arrays)
  constexpr char kCompilerVersion[]{"Clang " __clang_version__};
#elif defined(WB_COMPILER_GCC)
  const std::string kCompilerVersion{std::to_string(__GNUC__) + "." +
                                     std::to_string(__GNUC_MINOR__) + "." +
                                     std::to_string(__GNUC_PATCHLEVEL__)};
#else
  constexpr char kCompilerVersion[];
#error "Please, add your compiler build version here."
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
#endif  // WB_OS_POSIX

#ifdef WB_OS_WIN
  G3LOG(INFO) << app_description << " v." << WB_PRODUCT_FILEVERSION_INFO_STRING
              << " build with MSVC " << _MSC_FULL_VER << '.' << _MSC_BUILD
              << " running on " << wb::base::win::GetVersion() << ".";
#endif
}

/**
 * @brief Setup heap allocator.
 */
void BootHeapMemoryAllocator() noexcept {
#ifdef WB_OS_WIN
  {
    // Terminate the app if system detected heap corruption.
    const auto error_code =
        wb::base::win::memory::EnableTerminationOnHeapCorruption();
    G3PLOGE2_IF(WARNING, error_code)
        << "Can't enable 'Terminate on Heap corruption' os feature, continue "
           "without it.";
  }

  {
    // Optimize heap caches now.
    const auto error_code = wb::base::win::memory::OptimizeHeapResourcesNow();
    G3PLOGE2_IF(WARNING, error_code)
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
 * @brief Makes fatal dialog context.
 * @param bootmgr_args Boot manager arguments.
 * @return Fatal dialog context.
 */
[[nodiscard]] wb::ui::FatalDialogContext MakeFatalContext(
    const wb::boot_manager::BootmgrArgs& bootmgr_args) noexcept {
#ifdef WB_OS_POSIX
  return wb::ui::FatalDialogContext{bootmgr_args.intl.Layout()};
#elif defined(WB_OS_WIN)
  return {bootmgr_args.intl, bootmgr_args.intl.Layout(),
          bootmgr_args.main_icon_id, bootmgr_args.small_icon_id};
#else
#error "Please define MakeFatalContext for your platform."
#endif
}

/**
 * Gets executable directory.
 * @param bootmgr_args Boot manager arguments.
 * @return Executable directory.
 */
[[nodiscard]] std::filesystem::path GetExecutableDirectoryPath(
    const wb::boot_manager::BootmgrArgs& bootmgr_args) noexcept {
  using namespace wb::base;

  const auto app_path_result = std2::filesystem::get_executable_directory();
  if (const auto* rc = std2::get_error(app_path_result)) WB_ATTRIBUTE_UNLIKELY {
      const auto& intl = bootmgr_args.intl;
      wb::ui::FatalDialog(
          intl::l18n(intl, "Boot Manager - Error"), *rc,
          intl::l18n(intl,
                     "Please, check app is installed correctly and you have "
                     "enough permissions to run it."),
          MakeFatalContext(bootmgr_args),
          intl::l18n(
              intl,
              "Can't get current directory.  Unable to load the kernel."));
    }

  return *std2::get_result(app_path_result);
}

/**
 * @brief Load and run kernel.
 * @param bootmgr_args Boot manager args.
 * @param intl Localization lookup.
 * @return App exit code.
 */
int KernelStartup(const wb::boot_manager::BootmgrArgs& bootmgr_args) noexcept {
  using namespace wb::base;

  const auto app_directory_path = GetExecutableDirectoryPath(bootmgr_args);

#ifdef WB_OS_WIN
  const std::string kernel_path{
      (app_directory_path / "whitebox-kernel.dll").string()};
#else
  const std::string kernel_path{
      (app_directory_path /
       "libwhitebox-kernel.so." WB_PRODUCT_VERSION_INFO_STRING)
          .string()};
#endif

#ifdef WB_OS_WIN
  const unsigned kernel_load_flags{
      LOAD_WITH_ALTERED_SEARCH_PATH |
      (!bootmgr_args.command_line_flags.insecure_allow_unsigned_module_target
           ? LOAD_LIBRARY_REQUIRE_SIGNED_TARGET
           : 0U)};
#else
  const int kernel_load_flags{RTLD_LAZY | RTLD_LOCAL};
#endif

  const auto& intl = bootmgr_args.intl;
  const auto kernel_library =
      ScopedSharedLibrary::FromLibraryOnPath(kernel_path, kernel_load_flags);
  if (const auto* kernel_module = std2::get_result(kernel_library))
    WB_ATTRIBUTE_LIKELY {
      using WhiteBoxKernelMain = decltype(&KernelMain);
      // NOLINTNEXTLINE(modernize-avoid-c-arrays)
      constexpr char kKernelMainName[]{"KernelMain"};

      // Good, try to find and launch whitebox-kernel.
      const auto kernel_main_entry =
          kernel_module->GetAddressAs<WhiteBoxKernelMain>(kKernelMainName);
      if (const auto* kernel_main = std2::get_result(kernel_main_entry))
        WB_ATTRIBUTE_LIKELY {
#ifdef WB_OS_WIN
          return (*kernel_main)(
              {bootmgr_args.app_description, bootmgr_args.instance,
               bootmgr_args.show_window_flags, bootmgr_args.main_icon_id,
               bootmgr_args.small_icon_id, bootmgr_args.intl});
#else
          return (*kernel_main)({bootmgr_args.app_description,
                                 bootmgr_args.argv, bootmgr_args.argc,
                                 bootmgr_args.intl});
#endif
        }

      wb::ui::FatalDialog(
          intl::l18n(intl, "Boot Manager - Error"),
          std::get<std::error_code>(kernel_main_entry),
          intl::l18n(intl,
                     "Please, check app is installed correctly and you have "
                     "enough permissions to run it."),
          MakeFatalContext(bootmgr_args),
          intl::l18n_fmt(intl, "Can't get '{0}' entry point from '{1}'.",
                         kKernelMainName, kernel_path));
    }
  else {
    wb::ui::FatalDialog(
        intl::l18n(intl, "Boot Manager - Error"),
        std::get<std::error_code>(kernel_library),
        intl::l18n(intl,
                   "Please, check app is installed correctly and you have "
                   "enough permissions to run it."),
        MakeFatalContext(bootmgr_args),
        intl::l18n_fmt(intl, "Can't load whitebox kernel '{0}'.", kernel_path));
  }
}

}  // namespace

/**
 * @brief Bootmgr entry point on Windows.
 * @param bootmgr_args Bootmgr args.
 * @return 0 on success.
 */
extern "C" [[nodiscard]] WB_BOOT_MANAGER_API int BootmgrMain(
    const wb::boot_manager::BootmgrArgs& bootmgr_args) {
  using namespace wb::base;

  DumpSystemInformation(bootmgr_args.app_description);

#ifdef WB_OS_WIN
  using namespace wb::base::win;

  // Requires Windows Version 1903+ for UTF-8 process code page.
  //
  // "With a minimum target version of Windows Version 1903, the process code
  // page will always be UTF-8 so legacy code page detection and conversion can
  // be avoided.".
  //
  // See
  // https://docs.microsoft.com/en-us/windows/uwp/design/globalizing/use-utf8-code-page#set-a-process-code-page-to-utf-8
  if (win::GetVersion() < win::Version::WIN10_19H1) WB_ATTRIBUTE_UNLIKELY {
      wb::ui::FatalDialog(
          intl::l18n(bootmgr_args.intl, "Boot Manager - Error"),
          std2::system_last_error_code(ERROR_OLD_WIN_VERSION),
          intl::l18n(bootmgr_args.intl,
                     "Please, update Windows to Windows 10, version 1903 (May "
                     "19, 2019) or greater."),
          MakeFatalContext(bootmgr_args),
          intl::l18n(
              bootmgr_args.intl,
              "Windows is too old.  At least Windows 10, version 1903 (May 19, "
              "2019)+ required."));
    }

  // Enable process attacks mitigation policies in scope.
  const auto scoped_process_mitigation_policies =
      security::ScopedProcessMitigationPolicies::New();
  G3PLOGE_IF(WARNING, std2::get_error(scoped_process_mitigation_policies))
      << "Can't enable process attacks mitigation policies, attacker can use "
         "system features to break in app.";

  // Handle call of CRT function with bad arguments on the thread.
  const error_handling::ScopedThreadInvalidParameterHandler
      scoped_thread_invalid_parameter_handler{
          error_handling::DefaultThreadInvalidParameterHandler};
  // Handle pure virtual function call.
  const error_handling::ScopedProcessPureCallHandler
      scoped_process_pure_call_handler{error_handling::DefaultPureCallHandler};
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

#ifdef WB_OS_WIN
  const std2::native_thread_name new_thread_name{L"WhiteBoxMain"};

  // Mark main thread with name to simplify debugging.
  const auto scoped_thread_name =
      std2::this_thread::ScopedThreadName::New(new_thread_name);
  G3PLOGE_IF(WARNING, std2::get_error(scoped_thread_name))
      << "Can't rename main thread, continue with default name.";
#else
  // Well, we can't just use this one, as it is shown in top and monitors, so
  // should be the same as app name.  Decided to use default app name for now.
  // const std2::native_thread_name new_thread_name{"WhiteBoxMain"};
#endif

  // Check only single instance of the app is running.
  const wb::boot_manager::ScopedAppInstanceManager scoped_app_instance_manager{
      bootmgr_args.app_description};
  const auto other_instance_status = scoped_app_instance_manager.GetStatus();
  if (other_instance_status ==
      wb::boot_manager::AppInstanceStatus::kAlreadyRunning) {
#ifdef WB_OS_WIN
    using namespace std::chrono_literals;

    const std::string window_class_name{
        wb::kernel::MainWindow::ClassName(bootmgr_args.app_description)};
    // Time to flash other instance window.
    wb::ui::win::FlashWindowByClass(window_class_name, 900ms);
#endif

    wb::ui::FatalDialog(
        intl::l18n(bootmgr_args.intl, "Boot Manager - Error"),
        std2::posix_last_error_code(EEXIST),
        intl::l18n_fmt(bootmgr_args.intl,
                       "Sorry, only single '{0}' can run at a time.",
                       bootmgr_args.app_description),
        MakeFatalContext(bootmgr_args),
        intl::l18n_fmt(bootmgr_args.intl,
                       "Can't run multiple copies of '{0}' at once.  Please, "
                       "stop existing copy or return to the game.",
                       bootmgr_args.app_description));
  }

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
      win::ScopedTimerResolution::New(std::chrono::milliseconds{
          bootmgr_args.command_line_flags.periodic_timer_resolution_ms});
  G3LOG_IF(WARNING, !!std::get_if<unsigned>(&scoped_minimum_timer_resolution))
      << "Failed to set minimum periodic timers resolution to "
      << bootmgr_args.command_line_flags.periodic_timer_resolution_ms
      << "ms, will run with default system one.  Error code: "
      << std::get<unsigned>(scoped_minimum_timer_resolution)
      << ".  See "
         "https://docs.microsoft.com/en-us/windows/win32/api/timeapi/"
         "nf-timeapi-timebeginperiod";

  {
    // Signal Multimedia Class Scheduler Service we have game thread here, so
    // utilize as much of the CPU as possible without denying CPU resources to
    // lower-priority applications.
    const win::mmcss::ScopedMmcssThreadTask game_task{
        win::mmcss::KnownScopedMmcssThreadTaskName::kGames};
    const win::mmcss::ScopedMmcssThreadTask playback_task{
        win::mmcss::KnownScopedMmcssThreadTaskName::kPlayback};
    const auto scoped_mmcss_thread_controller =
        win::mmcss::ScopedMmcssThreadController::New(game_task, playback_task);
    if (const auto* controller =
            std2::get_result(scoped_mmcss_thread_controller))
      WB_ATTRIBUTE_LIKELY {
        const auto responsiveness_percent =
            controller->GetResponsivenessPercent();

        if (const auto* percent = std2::get_result(responsiveness_percent))
          WB_ATTRIBUTE_LIKELY {
            G3DLOG(INFO) << "Multimedia Class Scheduler Service uses "
                         << implicit_cast<unsigned>(*percent)
                         << "% of CPU for system wide tasks.";
          }
        else {
          G3PLOG_E(WARNING, *std2::get_error(responsiveness_percent))
              << "Can't get system responsiveness setting used by Multimedia "
                 "Class Scheduler Service for the main app thread.";
        }

        const auto bump_thread_priority_rc = controller->SetPriority(
            win::mmcss::ScopedMmcssThreadPriority::kHigh);
        G3PLOGE2_IF(WARNING, bump_thread_priority_rc)
            << "Can't get system responsiveness setting used by Multimedia "
               "Class Scheduler Service for the thread.";
      }
    else {
      G3PLOG_E(WARNING, *std2::get_error(scoped_mmcss_thread_controller))
          << "Can't enable Multimedia Class Scheduler Service for the app, "
             "some CPU resources may be underutilized.  See "
             "https://docs.microsoft.com/en-us/windows/win32/procthread/"
             "multimedia-class-scheduler-service#registry-settings";
    }
  }
#endif

  return KernelStartup(bootmgr_args);
}