// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Whitebox kernel main entry point.

#include "whitebox_kernel_main.h"

#include "base/deps/g3log/g3log.h"
#ifdef WB_OS_WIN
#include "base/win/ui/base_window.h"
#include "base/win/ui/peek_message_dispatcher.h"
#include "base/win/windows_light.h"
#include "whitebox-kernel/main_window_win.h"
#else
#include <chrono>
#include <thread>

#include "base/deps/sdl/sdl.h"
#include "build/static_settings_config.h"
#endif

namespace {
#ifdef WB_OS_WIN
/**
 * @brief Creates main app window definition.
 * @param kernel_args Kernel args.
 * @param window_title Window title.
 * @param width Window width.
 * @param height WIndow height.
 * @return Window definition.
 */
[[nodiscard]] wb::base::windows::ui::WindowDefinition
CreateMainWindowDefinition(const wb::kernel::KernelArgs& kernel_args,
                           const char* window_title, _In_ int width,
                           _In_ int height) noexcept {
  G3DCHECK(!!kernel_args.instance);

  const auto cursor = LoadCursor(nullptr, IDC_ARROW);
  return wb::base::windows::ui::WindowDefinition{
      kernel_args.instance, window_title, kernel_args.main_icon_id,
      kernel_args.small_icon_id, cursor,
      // TODO(dimhotepus): Remove when use Vulkan renderer?
      reinterpret_cast<HBRUSH>(GetStockObject(NULL_BRUSH)), WS_OVERLAPPEDWINDOW,
      0, CW_USEDEFAULT, CW_USEDEFAULT, width, height};
}

/**
 * @brief Run app message loop.
 * @return App exit code.
 */
[[nodiscard]] int DispatchMessages(
    _In_z_ const char* main_window_name) noexcept {
  int rc{0};
  bool is_done{false};
  const auto handle_quit_message = [&](const MSG& msg) noexcept {
    if (msg.message == WM_QUIT) {
      rc = static_cast<int>(msg.wParam);
      is_done = true;
    }
  };

  using namespace wb::base::windows;
  ui::PeekMessageDispatcher message_dispatcher;

  // Main message app loop.
  while (!is_done) {
    message_dispatcher.Dispatch(ui::HasNoPreDispatchMessage,
                                handle_quit_message);
  }

  G3LOG_IF(WARNING, rc != 0)
      << "Main window '" << main_window_name
      << "' message dispatch thread exited with non success code " << rc;

  return rc;
}
#else
/**
 * @brief Run app message loop.
 * @return App exit code.
 */
[[nodiscard]] int DispatchMessages() noexcept {
  SDL_Event event;
  bool is_done{false};

  while (!is_done) {
    while (::SDL_PollEvent(&event) == 1) {
      switch (event.type) {
        case SDL_QUIT:
          is_done = true;
          break;
      }
    }

    // TODO(dimhotepus): Do smth when no events.
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(5ms);
  }

  return 0;
}

/**
 * Dump fata error to log, show UI messsage box and exit.
 * @param title Message box title.
 * @param error Message box error.
 */
[[noreturn]] void FatalError(const char* title, const char* error) {
  G3LOG(WARNING) << error;

  std::string error_title{title};
  error_title += " Startup Error";

  ShowSimpleMessageBox(
      wb::sdl::MessageBoxFlags::Error | wb::sdl::MessageBoxFlags::LeftToRight,
      error_title.c_str(), error);

  std::exit(1);
}

[[nodiscard]] const char* GetWindowGraphicsContext(
    wb::sdl::SdlWindowFlags flags) noexcept {
  if ((flags & wb::sdl::SdlWindowFlags::kUseOpengl) ==
      wb::sdl::SdlWindowFlags::kUseOpengl) {
    return "OpenGL";
  }

  if ((flags & wb::sdl::SdlWindowFlags::kUseVulkan) ==
      wb::sdl::SdlWindowFlags::kUseVulkan) {
    return "Vulkan";
  }

  if ((flags & wb::sdl::SdlWindowFlags::kUseMetal) ==
      wb::sdl::SdlWindowFlags::kUseMetal) {
    return "Metal";
  }

  return "N/A";
};
#endif
}  // namespace

extern "C" [[nodiscard]] WB_WHITEBOX_KERNEL_API int KernelMain(
    const wb::kernel::KernelArgs& kernel_args) {
  using namespace wb::base;
  using namespace wb::kernel;

  // TODO(dimhotepus): Get screen size and use it if less than our minimal.
  constexpr int window_width{1024};
  constexpr int window_height{768};

#ifdef WB_OS_WIN
  const windows::ui::WindowDefinition window_definition{
      CreateMainWindowDefinition(kernel_args, kernel_args.app_description,
                                 window_width, window_height)};
  constexpr DWORD window_class_style{CS_HREDRAW | CS_VREDRAW};

  auto window_result = windows::ui::BaseWindow::New<MainWindow>(
      window_definition, window_class_style);
  if (auto* window_ptr = std_ext::GetSuccessResult(window_result);
      auto* window = window_ptr ? window_ptr->get() : nullptr) {
    // If the window was previously visible, the return value is nonzero.  If
    // the window was previously hidden, the return value is zero.
    window->Show(kernel_args.show_window_flags);
    // Send WM_PAINT directly to draw first time.
    window->Update();

    return DispatchMessages(window_definition.name);
  }

  const auto error_code = std::get<std::error_code>(window_result);
  G3PLOG_E(WARNING, error_code)
      << "Unable to create main '" << window_definition.name
      << "' window.  Please, contact authors.";

  return error_code.value();
#else
  using namespace wb::sdl;

  const ::SDL_version compiled_sdl_version{GetCompileTimeVersion()};
  const ::SDL_version linked_sdl_version{GetLinkTimeVersion()};

  const auto sdl_initializer = SdlInitializer::New(SdlInitializerFlags::kAudio |
                                                   SdlInitializerFlags::kVideo);
  if (GetSuccessResult(sdl_initializer) != nullptr) [[likely]] {
    G3LOG(INFO) << "SDL build v." << compiled_sdl_version << ", runtime v."
                << linked_sdl_version;
  } else {
    std::stringstream error_stream{std::ios_base::out};
    error_stream << "Unable to initialize SDL build/runtime v."
                 << compiled_sdl_version << "/v." << linked_sdl_version
                 << ", revision '" << ::SDL_GetRevision() << '\'' << ".\n\n"
                 << *GetErrorCode(sdl_initializer)
                 << ".\n\nPlease, fill the issue at "
                 << wb::build::settings::ui::error_dialog::kIssuesLink;

    FatalError(kernel_args.app_description, error_stream.str().c_str());
  }

  // TODO(dimhotepus): kAllowHighDpi requires usage of SDL_GetWindowSize() to
  // query the client area's size in screen coordinates, and
  // SDL_GL_GetDrawableSize() or SDL_GetRendererOutputSize() to query the
  // drawable size in pixels.
  SdlWindowFlags window_flags{SdlWindowFlags::kResizable |
                              SdlWindowFlags::kAllowHighDpi};

#if defined(WB_OS_LINUX)
  window_flags = window_flags | SdlWindowFlags::kUseOpengl;
#elif defined(WB_OS_MACOSX)
  window_flags = window_flags | SdlWindowFlags::kUseMetal;
#else
#error Unknown platform. Please, define SDL window flags for your platform in \
  whitebox-kernel/whitebox_kernel_main.cc
#endif

  const auto sdl_window = SdlWindow::New(
      kernel_args.app_description, SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED, window_width, window_height, window_flags);
  const auto* window = GetSuccessResult(sdl_window);
  if (!window) [[unlikely]] {
    std::stringstream error_stream{std::ios_base::out};
    error_stream << "Unable to create SDL window with "
                 << GetWindowGraphicsContext(window_flags) << " context."
                 << "\n\n: " << *GetErrorCode(sdl_window)
                 << ".\n\nMay be you forget to install "
                 << GetWindowGraphicsContext(window_flags) << " drivers?"
                 << "\n\nIf it is not the case, please, fill the issue at "
                 << wb::build::settings::ui::error_dialog::kIssuesLink;

    FatalError(kernel_args.app_description, error_stream.str().c_str());
  }

  {
    SDL_SysWMinfo platform_window_info;
    const auto rc = window->GetPlatformInfo(platform_window_info);
    if (rc.IsSucceeded()) [[likely]] {
      G3LOG(INFO) << "SDL window subsystem: " << platform_window_info.subsystem
                  << ".";
    } else {
      G3LOG(WARNING) << "Can't get platform specific SDL window info, error: "
                     << rc;
    }
  }

  return DispatchMessages();
#endif
}