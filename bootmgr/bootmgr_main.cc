// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Bootmgr main entry point.

#include "bootmgr_main.h"

#include <filesystem>

#include "base/deps/g3log/g3log.h"
#include "base/deps/mimalloc/mimalloc.h"
#include "base/scoped_floating_point_mode.h"
#include "base/scoped_thread_terminate_handler.h"
#include "base/threads/thread_utils.h"
#include "base/unique_module_ptr.h"
#include "base/windows/dll_load_utils.h"
#include "base/windows/error_handling/scoped_pure_call_handler.h"
#include "base/windows/error_handling/scoped_thread_invalid_parameter_handler.h"
#include "base/windows/error_handling/scoped_thread_unexpected_handler.h"
#include "base/windows/memory/memory_utils.h"
#include "base/windows/memory/scoped_new_handler.h"
#include "base/windows/memory/scoped_new_mode.h"
#include "base/windows/scoped_minimum_timer_resolution.h"
#include "base/windows/security/process_mitigations.h"
#include "base/windows/ui/task_dialog.h"
#include "base/windows/windows_version.h"
#include "build/static_settings_config.h"
#include "whitebox-kernel/whitebox_kernel_main.h"

namespace {
/**
 * @brief Setup heap allocator.
 */
void BootHeapMemoryAllocator() {
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

  // Ignore earlier allocations.
  mi_stats_reset();

  G3DLOG(INFO) << "Using mi-malloc v." << mi_version();
}

/**
 * @brief Load and run kernel.
 * @param bootmgr_args Boot manager args.
 * @return App exit code.
 */
int KernelStartup(const wb::bootmgr::BootmgrArgs& bootmgr_args) noexcept {
  using namespace wb::base;

  const auto app_path = windows::GetApplicationDirectory(bootmgr_args.instance);
  G3PLOGE_IF(FATAL, !!std::get_if<std::error_code>(&app_path),
             std::get<std::error_code>(app_path))
      << "Can't get current directory.  Unable to load "
         "the kernel.  Please, contact authors.";

  std::error_code rc;

  const std::string kernel_path{std::get<std::string>(app_path) +
                                "whitebox-kernel.dll"};
  const unsigned kernel_load_flags{
      LOAD_WITH_ALTERED_SEARCH_PATH |
      (windows::MustBeSignedDllLoadTarget(bootmgr_args.command_line)
           ? LOAD_LIBRARY_REQUIRE_SIGNED_TARGET
           : 0U)};

  const auto kernel_load_result =
      unique_module_ptr::from_load_library(kernel_path, kernel_load_flags);
  if (const auto* kernel_module =
          std::get_if<unique_module_ptr>(&kernel_load_result)) {
    using KernelMainFunction = decltype(&KernelMain);
    constexpr char kKernelMainFunctionName[]{"KernelMain"};

    // Good, try to find and launch whitebox-kernel.
    const auto kernel_main_result =
        kernel_module->get_address_as<KernelMainFunction>(
            kKernelMainFunctionName);
    if (const auto* kernel_main =
            std::get_if<KernelMainFunction>(&kernel_main_result)) {
      return (*kernel_main)(
          {bootmgr_args.instance, bootmgr_args.app_description,
           bootmgr_args.show_window_flags, bootmgr_args.main_icon_id,
           bootmgr_args.small_icon_id});
    }

    // TODO(dimhotepus): Show fancy Ui message box.
    rc = std::get<std::error_code>(kernel_main_result);
    G3PLOG_E(WARNING, rc)
        << "Can't get '" << kKernelMainFunctionName << "' entry point from '"
        << kernel_path
        << "'.  Looks like app is broken, please, reinstall the one.";
  } else {
    // TODO(dimhotepus): Show fancy Ui message box.
    rc = std::get<std::error_code>(kernel_load_result);
    G3PLOG_E(WARNING, rc) << "Can't load whitebox kernel '" << kernel_path
                          << "'.  Please, reinstall the app.";
  }

  return rc.value();
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
  using namespace wb::base::windows;

  // TODO(dimhotepus): Localize.
  constexpr char kBootmgrDialogTitle[]{"Boot Manager - Error"};
  constexpr char kSeeTechDetails[]{"See techical details"};
  constexpr char kHideTechDetails[]{"Hide techical details"};

  // Requires Windows Version 1903+ for UTF-8 process code page.
  //
  // "With a minimum target version of Windows Version 1903, the process code
  // page will always be UTF-8 so legacy code page detection and conversion can
  // be avoided.".
  //
  // See
  // https://docs.microsoft.com/en-us/windows/uwp/design/globalizing/use-utf8-code-page#set-a-process-code-page-to-utf-8
  if (windows::GetVersion() < windows::Version::WIN10_19H1) [[unlikely]] {
    constexpr char kOldWindowsVersionError[]{
        "Windows version is too old.  Version 1903 (May 2019 Update) or "
        "greater is required."};
    constexpr DWORD kOldWindowsVersionErrorCode{ERROR_OLD_WIN_VERSION};
    ui::DialogBoxSettings dialog_settings(
        nullptr, kBootmgrDialogTitle,
        "Update Windows to \"Version 1903 (May 2019 Update)\" or greater",
        kOldWindowsVersionError, kHideTechDetails, kSeeTechDetails,
        std_ext::GetThreadErrorCode(kOldWindowsVersionErrorCode).message(),
        wb::build::settings::ui::error_dialog::kFooterLink,
        ui::DialogBoxButton::kOk, bootmgr_args.main_icon_id,
        bootmgr_args.small_icon_id, false);
    ui::ShowDialogBox(ui::DialogBoxKind::kError, dialog_settings);

    G3PLOG_E(FATAL, std::error_code(kOldWindowsVersionErrorCode,
                                    std::system_category()))
        << kOldWindowsVersionError;

    return kOldWindowsVersionErrorCode;
  }

  // Enable process attacks mitigation policies in scope.
  const security::ScopedProcessMitigationPolicies
      scoped_process_mitigation_policies;
  G3PLOGE_IF(FATAL, !!scoped_process_mitigation_policies.error_code(),
             scoped_process_mitigation_policies.error_code())
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

  // Handle terminate function call on the thread.
  const ScopedThreadTerminateHandler scoped_thread_terminate_handler{
      DefaultThreadTerminateHandler};
  // Handle unexpected function call on the thread.
  const error_handling::ScopedThreadUnexpectedHandler
      scoped_thread_unexpected_handler{
          error_handling::DefaultThreadUnexpectedHandler};

  // Mark main thread with name to simplify debugging.
  const auto scoped_thread_name =
      threads::ScopedThreadName::New(::GetCurrentThread(), L"WhiteBoxMain");
  G3PLOGE_IF(WARNING, !!std::get_if<std::error_code>(&scoped_thread_name),
             std::get<std::error_code>(scoped_thread_name))
      << "Can't rename main thread, continue with default name.";

  // Setup heap memory allocator.
  BootHeapMemoryAllocator();

  // Ensure CPU floating point units convert denormals to zero and flush to zero
  // on underflow.
  const ScopedFloatingPointMode scoped_floating_point_mode{
      ScopedFloatingPointModeFlags::kDenormalsAreZero |
      ScopedFloatingPointModeFlags::kFlushToZero};

  // Set minimum timers resolution to good enough, but not too power hungry.
  const windows::ScopedMinimumTimerResolution scoped_minimum_timer_resolution{
      wb::build::settings::kMinimumTimerResolutionMs};
  G3LOG_IF(WARNING, !scoped_minimum_timer_resolution.IsSucceeded())
      << "Failed to set minimum periodic timers resolution to "
      << wb::build::settings::kMinimumTimerResolutionMs
      << "ms, will run with default system one.";

  return KernelStartup(bootmgr_args);
}