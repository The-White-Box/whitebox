// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Boot manager main entry point.

#include "main.h"

#include <filesystem>

#include "app_version_config.h"
#include "base/deps/abseil/cleanup/cleanup.h"
#include "base/deps/g3log/g3log.h"
#include "base/deps/marl/scheduler.h"
#include "base/deps/marl/scheduler_config.h"
#include "base/intl/l18n.h"
#include "base/scoped_floating_point_mode.h"
#include "base/scoped_process_terminate_handler.h"
#include "base/scoped_shared_library.h"
#include "base/std2/filesystem_ext.h"
#include "base/std2/system_error_ext.h"
#include "build/build_config.h"
#include "kernel/main.h"
#include "ui/fatal_dialog.h"

#ifdef WB_OS_WIN
#include "base/deps/abseil/cleanup/cleanup.h"
#include "base/scoped_app_instance_manager.h"
#include "base/std2/thread_ext.h"
#include "base/win/dll_load_utils.h"
#include "base/win/error_handling/scoped_process_pure_call_handler.h"
#include "base/win/error_handling/scoped_thread_invalid_parameter_handler.h"
#include "base/win/memory/memory_utils.h"
#include "base/win/memory/scoped_new_mode.h"
#include "base/win/mmcss/scoped_mmcss_thread_controller.h"
#include "base/win/scoped_timer_resolution.h"
#include "base/win/security/process_mitigations.h"
#include "base/win/windows_version.h"
#include "build/static_settings_config.h"
#include "kernel/main_window_win.h"
#include "ui/win/window_utilities.h"
#endif

#ifdef WB_OS_POSIX
#include <unistd.h>
#endif

namespace {

/**
 * @brief On Windows routine checks if the caller's process is a member of the
 * Administrators local group.  Caller is NOT expected to be impersonating
 * anyone and is expected to be able to open its own process and process token.
 * On *nix checks for uid equality to euid.
 * @return true if super user (root or admin), false otherwise.
 */
bool IsSuperUser() noexcept {
#ifdef WB_OS_WIN
  using namespace wb::base;

  PSID administrator_group_sid{nullptr};
  SID_IDENTIFIER_AUTHORITY nt_authority{SECURITY_NT_AUTHORITY};

  const auto free_sid_scope =
      absl::Cleanup([administrator_group_sid]() noexcept {
        if (administrator_group_sid) [[likely]] {
          // Not debug check!
          G3PCHECK_E(!::FreeSid(administrator_group_sid),
                     std2::system_last_error_code())
              << "FreeSid(administrator_group_sid) failed.";
        }
      });

  bool is_ok{!!::AllocateAndInitializeSid(
      &nt_authority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0,
      0, 0, 0, 0, 0, &administrator_group_sid)};
  G3DPCHECK_E(is_ok, std2::system_last_error_code())
      << "AllocateAndInitializeSid(SECURITY_NT_AUTHORITY, ...) failed.";

  if (is_ok) {
    BOOL is_administrator{TRUE};

    is_ok = !!::CheckTokenMembership(nullptr, administrator_group_sid,
                                     &is_administrator);
    G3DPCHECK_E(!!is_ok, std2::system_last_error_code())
        << "CheckTokenMembership(administrator_group_sid, ...) failed.";

    return is_administrator != FALSE;
  }

  return is_ok;
#elif defined(WB_OS_POSIX)
  const uid_t uid{getuid()}, euid{geteuid()};

  // We might have elevated privileges beyond that of the user who invoked the
  // program, due to suid bit.
  return euid == 0 || uid != euid;
#else
#error "Please define IsSuperUser for your platform."
#endif  // WB_OS_WIN || WB_OS_POSIX
}

/**
 * Dump some system information.
 * @param app_description Application description.
 * @param assets_path Assets path.
 */
void DumpSystemInformation(std::string_view app_description,
                           const std::string& assets_path) noexcept {
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
              << ", ABI stamp " << __GLIBCXX__ << " running with assets from '"
              << assets_path << "'.";
#endif
#ifdef _LIBCPP_VERSION
  G3LOG(INFO) << app_description << " v." << WB_PRODUCT_FILEVERSION_INFO_STRING
              << " build with " << kCompilerVersion << " on libc++ "
              << _LIBCPP_VERSION << "/ ABI " << _LIBCPP_ABI_VERSION
              << " running with assets from '" << assets_path << "'.";
#endif
#endif  // WB_OS_POSIX

#ifdef WB_OS_WIN
  G3LOG(INFO) << app_description << " v." << WB_PRODUCT_FILEVERSION_INFO_STRING
              << " build with MSVC " << _MSC_FULL_VER << '.' << _MSC_BUILD
              << " running on " << wb::base::win::GetVersion()
              << " with assets from '" << assets_path << "'.";
#endif
}

/**
 * @brief Makes fatal dialog context.
 * @param boot_manager_args Boot manager arguments.
 * @return Fatal dialog context.
 */
[[nodiscard]] wb::ui::FatalDialogContext MakeFatalContext(
    const wb::boot_manager::BootManagerArgs& boot_manager_args) noexcept {
#ifdef WB_OS_POSIX
  return wb::ui::FatalDialogContext{boot_manager_args.intl.Layout()};
#elif defined(WB_OS_WIN)
  return {boot_manager_args.intl, boot_manager_args.intl.Layout(),
          boot_manager_args.main_icon_id, boot_manager_args.small_icon_id};
#else
#error "Please define MakeFatalContext for your platform."
#endif
}

/**
 * @brief Load and run kernel.
 * @param boot_manager_args Boot manager args.
 * @param intl Localization lookup.
 * @return App exit code.
 */
int KernelStartup(
    const wb::boot_manager::BootManagerArgs& boot_manager_args) noexcept {
  using namespace wb::base;

  const auto app_path_result = std2::filesystem::get_executable_directory();
  if (const std::error_code& rc = app_path_result.error_or(std2::ok_code))
      [[unlikely]] {
    const auto& intl = boot_manager_args.intl;
    return wb::ui::FatalDialog(
        intl::l18n(intl, "Boot Manager - Error"), rc,
        intl::l18n(intl,
                   "Please, check app is installed correctly and you have "
                   "enough permissions to run it."),
        MakeFatalContext(boot_manager_args),
        intl::l18n(intl,
                   "Can't get current directory.  Unable to load the kernel."));
  }

  const std::filesystem::path& app_directory_path = *app_path_result;

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
      (!boot_manager_args.command_line_flags
               .insecure_allow_unsigned_module_target
           ? LOAD_LIBRARY_REQUIRE_SIGNED_TARGET
           : 0U)};
#else
  const int kernel_load_flags{RTLD_LAZY | RTLD_LOCAL};
#endif

  const auto& intl = boot_manager_args.intl;
  const auto kernel_library =
      ScopedSharedLibrary::FromLibraryOnPath(kernel_path, kernel_load_flags);
  if (kernel_library.has_value()) [[likely]] {
    using WhiteBoxKernelMain = decltype(&KernelMain);
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    constexpr char kKernelMainName[]{"KernelMain"};

    // Good, try to find and launch whitebox-kernel.
    const auto kernel_main =
        kernel_library->GetAddressAs<WhiteBoxKernelMain>(kKernelMainName);
    if (kernel_main.has_value()) [[likely]] {
#ifdef WB_OS_WIN
      return (*kernel_main)(
          {boot_manager_args.app_description, boot_manager_args.instance,
           boot_manager_args.show_window_flags, boot_manager_args.main_icon_id,
           boot_manager_args.small_icon_id,
           boot_manager_args.command_line_flags, boot_manager_args.intl});
#else
      return (*kernel_main)({boot_manager_args.app_description,
                             boot_manager_args.command_line_flags,
                             boot_manager_args.intl});
#endif
    }

    return wb::ui::FatalDialog(
        intl::l18n(intl, "Boot Manager - Error"), kernel_main.error(),
        intl::l18n(intl,
                   "Please, check app is installed correctly and you have "
                   "enough permissions to run it."),
        MakeFatalContext(boot_manager_args),
        intl::l18n_fmt(intl, "Can't get '{0}' entry point from '{1}'.",
                       kKernelMainName, kernel_path));
  }

  return wb::ui::FatalDialog(
      intl::l18n(intl, "Boot Manager - Error"), kernel_library.error(),
      intl::l18n(intl,
                 "Please, check app is installed correctly and you have "
                 "enough permissions to run it."),
      MakeFatalContext(boot_manager_args),
      intl::l18n_fmt(intl, "Can't load whitebox kernel '{0}'.", kernel_path));
}

}  // namespace

/**
 * @brief Boot manager entry point on Windows.
 * @param boot_manager_args Boot manager args.
 * @return 0 on success.
 */
extern "C" [[nodiscard]] WB_BOOT_MANAGER_API int BootManagerMain(
    const wb::boot_manager::BootManagerArgs& boot_manager_args) {
  using namespace wb::base;

  DumpSystemInformation(boot_manager_args.app_description,
                        boot_manager_args.command_line_flags.assets_path);

  // Well, exploits are everywhere.  Try to reduce overall damage as much as we
  // can.
  if (IsSuperUser()) [[unlikely]] {
    return wb::ui::FatalDialog(
        intl::l18n(boot_manager_args.intl, "Boot Manager - Error"),
#ifdef WB_OS_WIN
        std2::system_last_error_code(ERROR_ACCESS_DENIED),
#elif defined(WB_OS_POSIX)
        std2::system_last_error_code(EPERM),
#endif
        intl::l18n(boot_manager_args.intl,
                   "Please, run app not as root / administrator. Priveleged "
                   "accounts are not supported."),
        MakeFatalContext(boot_manager_args),
        intl::l18n(boot_manager_args.intl,
                   "Your user account is root or administrator. Running app as "
                   "root or administrator have security risks."));
  }

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
  if (win::GetVersion() < win::Version::WIN10_19H1) [[unlikely]] {
    return wb::ui::FatalDialog(
        intl::l18n(boot_manager_args.intl, "Boot Manager - Error"),
        std2::system_last_error_code(ERROR_OLD_WIN_VERSION),
        intl::l18n(boot_manager_args.intl,
                   "Please, update Windows to Windows 10, version 1903 (May "
                   "19, 2019) or greater."),
        MakeFatalContext(boot_manager_args),
        intl::l18n(
            boot_manager_args.intl,
            "Windows is too old.  At least Windows 10, version 1903 (May 19, "
            "2019)+ required."));
  }

  // Enable process attacks mitigation policies in scope.
  const auto scoped_process_mitigation_policies =
      security::ScopedProcessMitigationPolicies::New();
  G3PLOGE2_IF(WARNING,
              scoped_process_mitigation_policies.error_or(std2::ok_code))
      << "Can't enable process attacks mitigation policies, attacker can use "
         "system features to break in app.";

  // Handle call of CRT function with bad arguments on the thread.
  const error_handling::ScopedThreadInvalidParameterHandler
      scoped_thread_invalid_parameter_handler{
          error_handling::DefaultThreadInvalidParameterHandler};
  // Handle pure virtual function call.
  const error_handling::ScopedProcessPureCallHandler
      scoped_process_pure_call_handler{error_handling::DefaultPureCallHandler};

#ifndef WB_MI_MALLOC
  // Call new when malloc failed.  Only works with default allocator (!=
  // mimalloc) for now.
  const memory::ScopedNewMode scoped_new_mode{
      memory::ScopedNewModeFlag::CallNew};
#endif  // !WB_MI_MALLOC
#endif  // WB_OS_WIN

  // Handle terminate function call on the thread.
  const ScopedProcessTerminateHandler scoped_process_terminate_handler{
      DefaultProcessTerminateHandler};

#ifdef WB_OS_WIN
  const std2::native_thread_name new_thread_name{"WhiteBox_Main"};

  // Mark main thread with name to simplify debugging.
  const auto scoped_thread_name =
      std2::this_thread::ScopedThreadName::New(new_thread_name);
  G3PLOGE2_IF(WARNING, scoped_thread_name.error_or(std2::ok_code))
      << "Can't rename main thread, continue with default name.";
#else
  // Well, we can't just use this one, as it is shown in top and monitors, so
  // should be the same as app name.  Decided to use default app name for now.
  // const std2::native_thread_name new_thread_name{"WhiteBoxMain"};
#endif

#ifdef WB_OS_WIN
  // Check only single instance of the app is running.  Windows only here
  // because on Linux / MacOS we want to show fatal dialog when app has icon,
  // hense postpone check till SDL window created and app icon set.
  const ScopedAppInstanceManager scoped_app_instance_manager{
      boot_manager_args.app_description};
  const auto other_instance_status = scoped_app_instance_manager.GetStatus();
  if (other_instance_status == AppInstanceStatus::kAlreadyRunning)
      [[unlikely]] {
    using namespace std::chrono_literals;

    const std::string window_class_name{
        wb::kernel::MainWindow::ClassName(boot_manager_args.app_description)};
    // Well, notify user about other instance window.
    wb::ui::win::FlashWindowByClass(window_class_name, 900ms);

    return wb::ui::FatalDialog(
        intl::l18n(boot_manager_args.intl, "Boot Manager - Error"),
        std2::posix_last_error_code(EEXIST),
        intl::l18n_fmt(boot_manager_args.intl,
                       "Sorry, only single '{0}' can run at a time.",
                       boot_manager_args.app_description),
        MakeFatalContext(boot_manager_args),
        intl::l18n_fmt(boot_manager_args.intl,
                       "Can't run multiple copies of '{0}' at once.  Please, "
                       "stop existing copy or return to the game.",
                       boot_manager_args.app_description));
  }
#endif

  // Ensure CPU floating point units convert denormals to zero.
  const ScopedFloatDenormalsAreZeroMode scoped_float_daz_mode{
      ScopedFloatDenormalsAreZeroFlags::kDenormalsAreZeroOn};

  // Ensure CPU floating point units flush to zero on underflow.
  const ScopedFloatFlushToZeroMode scoped_float_ftz_mode{
      ScopedFloatFlushToZeroFlags::kFlushToZeroOn};

#ifdef WB_OS_WIN
  // Set minimum timers resolution to good enough, but not too power hungry.
  const auto scoped_minimum_timer_resolution =
      win::ScopedTimerResolution::New(std::chrono::milliseconds{
          boot_manager_args.command_line_flags.periodic_timer_resolution_ms});
  G3LOG_IF(WARNING, !scoped_minimum_timer_resolution.has_value())
      << "Failed to set minimum periodic timers resolution to "
      << boot_manager_args.command_line_flags.periodic_timer_resolution_ms
      << "ms, will run with default system one.  Error code: "
      << scoped_minimum_timer_resolution.error()
      << ".  See "
         "https://docs.microsoft.com/en-us/windows/win32/api/timeapi/"
         "nf-timeapi-timebeginperiod";

  {
    using namespace win::mmcss;

    // Signal Multimedia Class Scheduler Service we have game thread here, so
    // utilize as much of the CPU as possible without denying CPU resources to
    // lower-priority applications.
    const auto scoped_mmcss_thread_controller =
        ScopedMmcssThreadController::New(
            ScopedMmcssThreadTask{KnownScopedMmcssThreadTaskName::kGames},
            ScopedMmcssThreadTask{KnownScopedMmcssThreadTaskName::kPlayback});
    if (scoped_mmcss_thread_controller.has_value()) [[likely]] {
      const auto responsiveness_percent =
          scoped_mmcss_thread_controller->GetResponsivenessPercent();

      if (responsiveness_percent.has_value()) [[likely]] {
        G3DLOG(INFO) << "Multimedia Class Scheduler Service uses "
                     << implicit_cast<unsigned>(*responsiveness_percent)
                     << "% of CPU for system wide tasks.";
      } else {
        G3PLOG_E(WARNING, responsiveness_percent.error())
            << "Can't get system responsiveness setting used by Multimedia "
               "Class Scheduler Service for the main app thread.";
      }

      const auto bump_thread_priority_rc =
          scoped_mmcss_thread_controller->SetPriority(
              ScopedMmcssThreadPriority::kHigh);
      G3PLOGE2_IF(WARNING, bump_thread_priority_rc)
          << "Can't set high priority for the thread in Multimedia Class "
             "Scheduler Service.";
    } else {
      G3PLOG_E(WARNING, scoped_mmcss_thread_controller.error())
          << "Can't enable Multimedia Class Scheduler Service for the app, "
             "some CPU resources may be underutilized.  See "
             "https://docs.microsoft.com/en-us/windows/win32/procthread/"
             "multimedia-class-scheduler-service#registry-settings";
    }
  }
#endif

  const unsigned logical_cores_num{marl::Thread::numLogicalCPUs()};
  const marl::Scheduler::Config all_cores_config =
      marl::Scheduler::Config()
          // Use all logical cores.
          .setWorkerThreadCount(static_cast<int>(logical_cores_num))
          // Setup all required thread state stuff.
          .setWorkerThreadStatefulInitializer(
              deps::marl::make_thread_start_state);

  // Create a marl scheduler and bind it to the main thread so we can call
  // marl::schedule()
  marl::Scheduler process_wide_scheduler{all_cores_config};
  process_wide_scheduler.bind();

  // Need to unbind scheduler from main thread.  Forgetting to unbind will
  // result in the marl::Scheduler destructor blocking indefinitely.
  const absl::Cleanup unbind_scheduler{
      [&]() noexcept { process_wide_scheduler.unbind(); }};

  G3LOG(INFO) << "Marl CPU scheduler using " << logical_cores_num
              << " logical cores.";

  return KernelStartup(boot_manager_args);
}