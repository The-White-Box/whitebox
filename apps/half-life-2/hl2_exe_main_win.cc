// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// The entry point for windows Half-Life 2 process.

#include <system_error>

#include "apps/args_win.h"
#include "apps/boot_heap_allocator.h"
#include "apps/cpu_feature_checks.h"
#include "apps/i18n_creator.h"
#include "apps/parse_command_line.h"
#include "base/deps/abseil/flags/flag.h"
#include "base/deps/abseil/strings/str_cat.h"
#include "base/deps/abseil/strings/str_join.h"
#include "base/deps/g3log/scoped_g3log_initializer.h"
#include "base/intl/l18n.h"
#include "base/intl/lookup.h"
#include "base/intl/scoped_process_locale.h"
#include "base/scoped_shared_library.h"
#include "base/win/com/scoped_com_fatal_exception_handler.h"
#include "base/win/com/scoped_com_strong_unmarshalling_policy.h"
#include "base/win/com/scoped_thread_com_initializer.h"
#include "base/win/dll_load_utils.h"
#include "base/win/error_handling/scoped_thread_error_mode.h"
#include "boot-manager/main.h"
#include "build/compiler_config.h"  // WB_ATTRIBUTE_DLL_EXPORT
#include "build/static_settings_config.h"
#include "hl2_exe_flags.h"
#include "resource_win.h"
#include "ui/fatal_dialog.h"

#ifdef WB_MI_MALLOC
#include "base/deps/mimalloc/scoped_dump_mimalloc_main_stats.h"
#endif

extern "C" {
// Starting with the Release 302 drivers, application developers can direct the
// Nvidia Optimus driver at runtime to use the High Performance Graphics to
// render any application–even those applications for which there is no existing
// application profile.
//
// See
// https://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
WB_ATTRIBUTE_DLL_EXPORT DWORD NvOptimusEnablement = 0x00000001;

// This will select the high performance AMD GPU as long as no profile exists
// that assigns the application to another GPU.  Please make sure to use a 13.35
// or newer driver.  Older drivers do not support this.
//
// See
// https://community.amd.com/t5/firepro-development/can-an-opengl-app-default-to-the-discrete-gpu-on-an-enduro/td-p/279440
WB_ATTRIBUTE_DLL_EXPORT int AmdPowerXpressRequestHighPerformance = 0x00000001;
}

namespace {

/**
 * @brief Makes fatal dialog context.
 * @param l18n Localization service.
 * @return Fatal dialog context.
 */
[[nodiscard]] wb::ui::FatalDialogContext MakeFatalContext(
    const wb::base::intl::LookupWithFallback& l18n) noexcept {
  return {l18n, l18n.Layout(), WB_HALF_LIFE_2_IDI_MAIN_ICON,
          WB_HALF_LIFE_2_IDI_SMALL_ICON};
}

/**
 * @brief Load and run boot manager.
 * @param instance App instance.
 * @param positional_flags Command line args which are not part of any parsed
 * flags.
 * @param show_window_flags Show window flags.
 * @param intl Localization lookup.
 * @return App exit code.
 */
int BootManagerStartup(
    _In_ HINSTANCE instance, _In_ std::vector<char*> positional_flags,
    _In_ int show_window_flags,
    _In_ const wb::base::intl::LookupWithFallback& intl) noexcept {
  using namespace wb::base;

  // Search for DLLs in the secure order to prevent DLL plant attacks.
  std::error_code rc{win::get_error(::SetDefaultDllDirectories(
      LOAD_LIBRARY_SEARCH_SYSTEM32 | LOAD_LIBRARY_SEARCH_USER_DIRS))};
  G3PLOGE2_IF(WARNING, rc)
      << "Can't enable secure DLL search order, attacker can plant DLLs with "
         "malicious code.";

  const auto app_path_result = win::GetApplicationDirectory(instance);
  if (const auto* error = std2::get_error(app_path_result))
    WB_ATTRIBUTE_UNLIKELY {
      wb::ui::FatalDialog(
          intl::l18n_fmt(intl, "{0} - Error",
                         WB_PRODUCT_FILE_DESCRIPTION_STRING),
          *error,
          intl::l18n(intl,
                     "Please, check app is installed correctly and you have "
                     "enough permissions to run it."),
          MakeFatalContext(intl),
          intl::l18n(intl,
                     "Can't get current directory.  May be app located too "
                     "deep (> 1024)?"));
    }

  const auto app_path = std2::get_result(app_path_result);
  G3DCHECK(!!app_path);

  const std::string boot_manager_path{*app_path + "whitebox-boot-manager.dll"};
  const std::uint32_t attempts_to_retry_allocate_memory{
      absl::GetFlag(FLAGS_attempts_to_retry_allocate_memory)};
  const wb::apps::flags::PeriodicTimerResolution periodic_timer_resolution{
      absl::GetFlag(FLAGS_periodic_timer_resolution_ms)};
  const wb::apps::flags::WindowWidth main_window_width{
      absl::GetFlag(FLAGS_main_window_width)};
  const wb::apps::flags::WindowHeight main_window_height{
      absl::GetFlag(FLAGS_main_window_height)};
  const bool insecure_allow_unsigned_module_target{
      absl::GetFlag(FLAGS_insecure_allow_unsigned_module_target)};
  const bool should_dump_heap_allocator_statistics_on_exit{
      absl::GetFlag(FLAGS_should_dump_heap_allocator_statistics_on_exit)};
  const unsigned boot_manager_flags{LOAD_WITH_ALTERED_SEARCH_PATH |
                                    (!insecure_allow_unsigned_module_target
                                         ? LOAD_LIBRARY_REQUIRE_SIGNED_TARGET
                                         : 0U)};

#ifdef WB_MI_MALLOC
  // Dumps mimalloc stats on exit?
  const wb::mi::ScopedDumpMiMainStats scoped_dump_mi_main_stats{
      should_dump_heap_allocator_statistics_on_exit};
#endif  // WB_MI_MALLOC

  const auto boot_manager_library = ScopedSharedLibrary::FromLibraryOnPath(
      boot_manager_path, boot_manager_flags);
  if (const auto* boot_manager = std2::get_result(boot_manager_library))
    WB_ATTRIBUTE_LIKELY {
      using BootManagerMain = decltype(&BootManagerMain);
      // NOLINTNEXTLINE(modernize-avoid-c-arrays)
      constexpr char kBootManagerMainName[]{"BootManagerMain"};

      // Good, try to find and launch boot manager.
      const auto boot_manager_entry =
          boot_manager->GetAddressAs<BootManagerMain>(kBootManagerMainName);
      if (const auto* boot_manager_main = std2::get_result(boot_manager_entry))
        WB_ATTRIBUTE_LIKELY {
          return (*boot_manager_main)(
              {instance,
               WB_PRODUCT_FILE_DESCRIPTION_STRING,
               show_window_flags,
               WB_HALF_LIFE_2_IDI_MAIN_ICON,
               WB_HALF_LIFE_2_IDI_SMALL_ICON,
               {.positional_flags = std::move(positional_flags),
                .attempts_to_retry_allocate_memory =
                    attempts_to_retry_allocate_memory,
                .periodic_timer_resolution_ms = periodic_timer_resolution.ms,
                .main_window_width = main_window_width.size,
                .main_window_height = main_window_height.size,
                .insecure_allow_unsigned_module_target =
                    insecure_allow_unsigned_module_target,
                .should_dump_heap_allocator_statistics_on_exit =
                    should_dump_heap_allocator_statistics_on_exit},
               intl});
        }

      wb::ui::FatalDialog(
          intl::l18n_fmt(intl, "{0} - Error",
                         WB_PRODUCT_FILE_DESCRIPTION_STRING),
          std::get<std::error_code>(boot_manager_entry),
          intl::l18n(intl,
                     "Please, check app is installed correctly and you have "
                     "enough permissions to run it."),
          MakeFatalContext(intl),
          intl::l18n_fmt(intl, "Can't get '{0}' entry point from '{1}'.",
                         kBootManagerMainName, boot_manager_path));
    }
  else {
    wb::ui::FatalDialog(
        intl::l18n_fmt(intl, "{0} - Error", WB_PRODUCT_FILE_DESCRIPTION_STRING),
        std::get<std::error_code>(boot_manager_library),
        intl::l18n(intl,
                   "Please, check app is installed correctly and you have "
                   "enough permissions to run it."),
        MakeFatalContext(intl),
        intl::l18n_fmt(intl, "Can't load boot manager '{0}'.",
                       boot_manager_path));
  }
}

}  // namespace

/**
 * @brief Windows app entry point.
 * @param instance App instance.
 * @param command_line Command line.
 * @param show_window_flags Show window flags.
 * @return App exit code.
 */
int WINAPI WinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE,
                   _In_ [[maybe_unused]] char* command_line,
                   _In_ int show_window_flags) {
  using namespace wb::base;
  using namespace wb::base::win;

#ifdef _DEBUG
  // Simplifies debugging experience, no need to sign targets.
  // Should never use GetCommandLine{A, W} anywhere in the app, or abstraction
  // leaks.
  char* full_command_line_ansi{::GetCommandLineA()};
  std::string debug_full_command_line_ansi{full_command_line_ansi};
  debug_full_command_line_ansi.append(
      " --insecure_allow_unsigned_module_target");
  full_command_line_ansi = debug_full_command_line_ansi.data();

  wchar_t* full_command_line_wide{::GetCommandLineW()};
  std::wstring debug_full_command_line_wide{full_command_line_wide};
  debug_full_command_line_wide.append(
      L" --insecure_allow_unsigned_module_target");
  full_command_line_wide = debug_full_command_line_wide.data();
#else
  const char* full_command_line_ansi{::GetCommandLineA()};
  const wchar_t* full_command_line_wide{::GetCommandLineW()};
#endif

  // Initialize g3log logging library first as logs are used extensively.
  const deps::g3log::ScopedG3LogInitializer scoped_g3log_initializer{
      full_command_line_ansi, wb::build::settings::kPathToMainLogFile};

  // Install heap allocator tracing / set options.
  wb::apps::BootHeapAllocator();

  // Start with specifying UTF-8 locale for all user-facing data.
  const intl::ScopedProcessLocale scoped_process_locale{
      intl::ScopedProcessLocaleCategory::kAll, intl::locales::kUtf8Locale};
  const auto l18n = wb::apps::CreateIntl(WB_PRODUCT_FILE_DESCRIPTION_STRING,
                                         scoped_process_locale);

  // Query CPU support for required features.  In case any required feature is
  // missed we return all required features with support state.
  const std::vector<wb::apps::CpuFeature> cpu_features_support{
      wb::apps::QueryRequiredCpuFeatures()};
  if (!cpu_features_support.empty()) WB_ATTRIBUTE_UNLIKELY {
      const std::string cpu_features_support_state{absl::StrJoin(
          cpu_features_support, "\n",
          [&](std::string* out, const wb::apps::CpuFeature& feature_support) {
            absl::StrAppend(
                out, intl::l18n_fmt(l18n, "{0}     {1}", feature_support.name,
                                    feature_support.is_supported ? "✓" : "❌"));
          })};
      wb::ui::FatalDialog(
          intl::l18n_fmt(l18n, "{0} - Error",
                         WB_PRODUCT_FILE_DESCRIPTION_STRING),
          std2::system_last_error_code(ERROR_DEVICE_HARDWARE_ERROR),
          intl::l18n(l18n,
                     "Sorry, your CPU has missed some required features to run "
                     "the game."),
          MakeFatalContext(l18n),
          intl::l18n_fmt(l18n, "CPU features support table for {0}:\n{1}",
                         wb::apps::QueryCpuBrand(),
                         cpu_features_support_state));
    }

  // Initialize command line flags.
  auto args_parse_result =
      wb::apps::win::Args::FromCommandLine(full_command_line_wide);
  if (const auto* error = std2::get_error(args_parse_result))
    WB_ATTRIBUTE_UNLIKELY {
      wb::ui::FatalDialog(
          intl::l18n_fmt(l18n, "{0} - Error",
                         WB_PRODUCT_FILE_DESCRIPTION_STRING),
          *error,
          intl::l18n(l18n,
                     "Please ensure you have enough free memory and use "
                     "command line correctly."),
          MakeFatalContext(l18n),
          intl::l18n(l18n,
                     "Can't parse command line flags.  See log for details."));
    }

  auto args = std2::get_result(args_parse_result);
  G3DCHECK(!!args);

  // Setup command line flags as they are used early.
  std::vector<char*> positional_flags{wb::apps::ParseCommandLine(
      args->count(), args->values(),
      {.app_name = WB_PRODUCT_FILE_DESCRIPTION_STRING,
       .app_version = WB_PRODUCT_FILE_VERSION_INFO_STRING,
       .app_usage = wb::apps::half_life_2::kUsageMessage})};

  // Calling thread will handle critical errors, does not show general
  // protection fault error box and message box when OpenFile failed to find
  // file.
  const auto scoped_thread_error_mode =
      error_handling::ScopedThreadErrorMode::New(
#ifdef NDEBUG
          error_handling::ScopedThreadErrorModeFlags::kFailOnCriticalErrors |
#endif
          error_handling::ScopedThreadErrorModeFlags::kNoGpFaultErrorBox |
          error_handling::ScopedThreadErrorModeFlags::kNoOpenFileErrorBox);
  G3PLOGE_IF(WARNING, std2::get_error(scoped_thread_error_mode))
      << "Can't set thread reaction to serious system errors, continue with "
         "default reaction.";

  // Initialize COM.  Required as ui::ShowDialogBox may call ShellExecute which
  // can delegate execution to shell extensions that are activated using COM.
  const auto scoped_thread_com_initializer =
      com::ScopedThreadComInitializer::New(
          com::ScopedThreadComInitializerFlags::kApartmentThreaded |
          com::ScopedThreadComInitializerFlags::kDisableOle1Dde |
          com::ScopedThreadComInitializerFlags::kSpeedOverMemory);
  G3PLOGE_IF(WARNING, std2::get_error(scoped_thread_com_initializer))
      << "Component Object Model initialization failed, continue without COM.";

  // Disable default COM exception swallowing, report all COM exceptions to us.
  const auto scoped_com_fatal_exception_handler =
      com::ScopedComFatalExceptionHandler::New();
  G3PLOGE_IF(WARNING, std2::get_error(scoped_com_fatal_exception_handler))
      << "Can't disable COM exceptions swallowing, some exceptions may not be "
         "passed to the app.";

  // Disallow COM marshalers and unmarshalers not from hardened system-trusted
  // per-process list.
  const auto scoped_com_strong_unmarshalling_policy =
      com::ScopedComStrongUnmarshallingPolicy::New();
  G3PLOGE_IF(WARNING, std2::get_error(scoped_com_strong_unmarshalling_policy))
      << "Can't enable strong COM unmarshalling policy, some non-trusted "
         "marshallers can be used.";

  return BootManagerStartup(instance, std::move(positional_flags),
                            show_window_flags, l18n);
}