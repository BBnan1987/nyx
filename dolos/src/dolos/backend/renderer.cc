#include "dolos/backend/renderer.h"

#include <lazy_importer.hpp>

namespace dolos {

#define WINDOW_CLASS_NAME "WTmpWindowEx"

TemporaryWindow::TemporaryWindow() : hwnd(nullptr) {
  // unregister just in case there is a dangling class from previous call
  LI_FN(UnregisterClassA)(WINDOW_CLASS_NAME, LI_FN(GetModuleHandleA)(nullptr));

  WNDCLASSEX wcx{};
  wcx.cbSize = sizeof(WNDCLASSEX);
  wcx.style = CS_HREDRAW | CS_VREDRAW;
  wcx.lpfnWndProc = (WNDPROC)LI_FN(GetProcAddress)(LI_FN(GetModuleHandleA)("user32.dll"), "DefWindowProcA");
  wcx.lpszClassName = WINDOW_CLASS_NAME;
  wcx.hInstance = LI_FN(GetModuleHandleA)(nullptr);

  if (!wcx.hInstance) {
    return;
  }

  if (!LI_FN(RegisterClassExA)(&wcx) && LI_FN(GetLastError)() != ERROR_ALREADY_REGISTERED) {
    return;
  }

  hwnd = LI_FN(CreateWindowExA)(NULL,
                                wcx.lpszClassName,
                                "tmp d3d wnd",
                                WS_OVERLAPPEDWINDOW,
                                0,
                                0,
                                100,
                                100,
                                nullptr,
                                nullptr,
                                wcx.hInstance,
                                nullptr);
  if (!hwnd) {
    return;
  }
}

TemporaryWindow::~TemporaryWindow() {
  if (hwnd) {
    LI_FN(DestroyWindow)(hwnd);
  }
  LI_FN(UnregisterClassA)(WINDOW_CLASS_NAME, LI_FN(GetModuleHandleA)(nullptr));
}

}  // namespace dolos
