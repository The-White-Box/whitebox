// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Whitebox kernel main entry point.

#include "whitebox_kernel_main.h"

#ifdef WB_OS_WIN
#include <tchar.h>
#endif

#include "base/deps/g3log/g3log.h"
#ifdef WB_OS_WIN
#include "base/windows/ui/base_window.h"
#include "base/windows/ui/peek_message_dispatcher.h"
#include "base/windows/windows_light.h"
#include "whitebox-kernel/main_window.h"
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
                           const std::string& window_title, _In_ int width,
                           _In_ int height) noexcept {
  G3DCHECK(!!kernel_args.instance);

  const auto cursor = LoadCursor(nullptr, IDC_ARROW);
  return wb::base::windows::ui::WindowDefinition{
      kernel_args.instance, window_title.c_str(), kernel_args.main_icon_id,
      kernel_args.small_icon_id, cursor,
      // TODO(dimhotepus): Remove when use Vulkan renderer?
      reinterpret_cast<HBRUSH>(GetStockObject(NULL_BRUSH)), WS_OVERLAPPEDWINDOW,
      0, CW_USEDEFAULT, CW_USEDEFAULT, width, height};
}

/**
 * @brief Run app message loop.
 * @return App exit code.
 */
int DispatchMessages(_In_z_ const char* main_window_name) noexcept {
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
#endif
}  // namespace

extern "C" [[nodiscard]] WB_WHITEBOX_KERNEL_API int KernelMain(
    const wb::kernel::KernelArgs& kernel_args) {
  using namespace wb::base;
  using namespace wb::kernel;

  const std::string window_title{std::string{kernel_args.app_description} +
                                 " [64 bit]"};

#ifdef WB_OS_WIN
  const windows::ui::WindowDefinition window_definition{
      CreateMainWindowDefinition(kernel_args, window_title, 640, 480)};
  constexpr DWORD window_class_style{CS_HREDRAW | CS_VREDRAW};

  auto window_result =
      MainWindow::Create(window_definition, window_class_style);
  if (auto* window_ptr = std::get_if<un<MainWindow>>(&window_result);
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
  G3LOG(WARNING) << "Unable to create main '" << window_title
                 << "' window.  Not implemented.";

  return 0;
#endif
}