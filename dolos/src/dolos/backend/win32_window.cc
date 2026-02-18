#include "dolos/backend/win32_window.h"

#include "dolos/game.h"
#include "dolos/main_thread.h"
#include "dolos/pipe_log.h"
#include "nyx/game_lock.h"
#include "nyx/imgui_input_event.h"
#include "nyx/nyx.h"
#include "nyx/nyx_imgui.h"
#include "nyx/util.h"

#include <imgui.h>
#include <windowsx.h>

#ifndef WM_MOUSEHWHEEL
#define WM_MOUSEHWHEEL 0x020E
#endif

#define IM_VK_KEYPAD_ENTER (VK_RETURN + 256)

static ImGuiKey VirtualKeyToImGuiKey(WPARAM wParam) {
  switch (wParam) {
    case VK_TAB:
      return ImGuiKey_Tab;
    case VK_LEFT:
      return ImGuiKey_LeftArrow;
    case VK_RIGHT:
      return ImGuiKey_RightArrow;
    case VK_UP:
      return ImGuiKey_UpArrow;
    case VK_DOWN:
      return ImGuiKey_DownArrow;
    case VK_PRIOR:
      return ImGuiKey_PageUp;
    case VK_NEXT:
      return ImGuiKey_PageDown;
    case VK_HOME:
      return ImGuiKey_Home;
    case VK_END:
      return ImGuiKey_End;
    case VK_INSERT:
      return ImGuiKey_Insert;
    case VK_DELETE:
      return ImGuiKey_Delete;
    case VK_BACK:
      return ImGuiKey_Backspace;
    case VK_SPACE:
      return ImGuiKey_Space;
    case VK_RETURN:
      return ImGuiKey_Enter;
    case VK_ESCAPE:
      return ImGuiKey_Escape;
    case VK_OEM_7:
      return ImGuiKey_Apostrophe;
    case VK_OEM_COMMA:
      return ImGuiKey_Comma;
    case VK_OEM_MINUS:
      return ImGuiKey_Minus;
    case VK_OEM_PERIOD:
      return ImGuiKey_Period;
    case VK_OEM_2:
      return ImGuiKey_Slash;
    case VK_OEM_1:
      return ImGuiKey_Semicolon;
    case VK_OEM_PLUS:
      return ImGuiKey_Equal;
    case VK_OEM_4:
      return ImGuiKey_LeftBracket;
    case VK_OEM_5:
      return ImGuiKey_Backslash;
    case VK_OEM_6:
      return ImGuiKey_RightBracket;
    case VK_OEM_3:
      return ImGuiKey_GraveAccent;
    case VK_CAPITAL:
      return ImGuiKey_CapsLock;
    case VK_SCROLL:
      return ImGuiKey_ScrollLock;
    case VK_NUMLOCK:
      return ImGuiKey_NumLock;
    case VK_SNAPSHOT:
      return ImGuiKey_PrintScreen;
    case VK_PAUSE:
      return ImGuiKey_Pause;
    case VK_NUMPAD0:
      return ImGuiKey_Keypad0;
    case VK_NUMPAD1:
      return ImGuiKey_Keypad1;
    case VK_NUMPAD2:
      return ImGuiKey_Keypad2;
    case VK_NUMPAD3:
      return ImGuiKey_Keypad3;
    case VK_NUMPAD4:
      return ImGuiKey_Keypad4;
    case VK_NUMPAD5:
      return ImGuiKey_Keypad5;
    case VK_NUMPAD6:
      return ImGuiKey_Keypad6;
    case VK_NUMPAD7:
      return ImGuiKey_Keypad7;
    case VK_NUMPAD8:
      return ImGuiKey_Keypad8;
    case VK_NUMPAD9:
      return ImGuiKey_Keypad9;
    case VK_DECIMAL:
      return ImGuiKey_KeypadDecimal;
    case VK_DIVIDE:
      return ImGuiKey_KeypadDivide;
    case VK_MULTIPLY:
      return ImGuiKey_KeypadMultiply;
    case VK_SUBTRACT:
      return ImGuiKey_KeypadSubtract;
    case VK_ADD:
      return ImGuiKey_KeypadAdd;
    case IM_VK_KEYPAD_ENTER:
      return ImGuiKey_KeypadEnter;
    case VK_LSHIFT:
      return ImGuiKey_LeftShift;
    case VK_LCONTROL:
      return ImGuiKey_LeftCtrl;
    case VK_LMENU:
      return ImGuiKey_LeftAlt;
    case VK_LWIN:
      return ImGuiKey_LeftSuper;
    case VK_RSHIFT:
      return ImGuiKey_RightShift;
    case VK_RCONTROL:
      return ImGuiKey_RightCtrl;
    case VK_RMENU:
      return ImGuiKey_RightAlt;
    case VK_RWIN:
      return ImGuiKey_RightSuper;
    case VK_APPS:
      return ImGuiKey_Menu;
    case '0':
      return ImGuiKey_0;
    case '1':
      return ImGuiKey_1;
    case '2':
      return ImGuiKey_2;
    case '3':
      return ImGuiKey_3;
    case '4':
      return ImGuiKey_4;
    case '5':
      return ImGuiKey_5;
    case '6':
      return ImGuiKey_6;
    case '7':
      return ImGuiKey_7;
    case '8':
      return ImGuiKey_8;
    case '9':
      return ImGuiKey_9;
    case 'A':
      return ImGuiKey_A;
    case 'B':
      return ImGuiKey_B;
    case 'C':
      return ImGuiKey_C;
    case 'D':
      return ImGuiKey_D;
    case 'E':
      return ImGuiKey_E;
    case 'F':
      return ImGuiKey_F;
    case 'G':
      return ImGuiKey_G;
    case 'H':
      return ImGuiKey_H;
    case 'I':
      return ImGuiKey_I;
    case 'J':
      return ImGuiKey_J;
    case 'K':
      return ImGuiKey_K;
    case 'L':
      return ImGuiKey_L;
    case 'M':
      return ImGuiKey_M;
    case 'N':
      return ImGuiKey_N;
    case 'O':
      return ImGuiKey_O;
    case 'P':
      return ImGuiKey_P;
    case 'Q':
      return ImGuiKey_Q;
    case 'R':
      return ImGuiKey_R;
    case 'S':
      return ImGuiKey_S;
    case 'T':
      return ImGuiKey_T;
    case 'U':
      return ImGuiKey_U;
    case 'V':
      return ImGuiKey_V;
    case 'W':
      return ImGuiKey_W;
    case 'X':
      return ImGuiKey_X;
    case 'Y':
      return ImGuiKey_Y;
    case 'Z':
      return ImGuiKey_Z;
    case VK_F1:
      return ImGuiKey_F1;
    case VK_F2:
      return ImGuiKey_F2;
    case VK_F3:
      return ImGuiKey_F3;
    case VK_F4:
      return ImGuiKey_F4;
    case VK_F5:
      return ImGuiKey_F5;
    case VK_F6:
      return ImGuiKey_F6;
    case VK_F7:
      return ImGuiKey_F7;
    case VK_F8:
      return ImGuiKey_F8;
    case VK_F9:
      return ImGuiKey_F9;
    case VK_F10:
      return ImGuiKey_F10;
    case VK_F11:
      return ImGuiKey_F11;
    case VK_F12:
      return ImGuiKey_F12;
    case VK_F13:
      return ImGuiKey_F13;
    case VK_F14:
      return ImGuiKey_F14;
    case VK_F15:
      return ImGuiKey_F15;
    case VK_F16:
      return ImGuiKey_F16;
    case VK_F17:
      return ImGuiKey_F17;
    case VK_F18:
      return ImGuiKey_F18;
    case VK_F19:
      return ImGuiKey_F19;
    case VK_F20:
      return ImGuiKey_F20;
    case VK_F21:
      return ImGuiKey_F21;
    case VK_F22:
      return ImGuiKey_F22;
    case VK_F23:
      return ImGuiKey_F23;
    case VK_F24:
      return ImGuiKey_F24;
    case VK_BROWSER_BACK:
      return ImGuiKey_AppBack;
    case VK_BROWSER_FORWARD:
      return ImGuiKey_AppForward;
    default:
      return ImGuiKey_None;
  }
}

namespace dolos {

Win32Window::Win32Window(Game* game, nyx::NyxImGui* nyx_imgui, nyx::GameLock* game_lock)
    : game_(game),
      nyx_imgui_(nyx_imgui),
      game_lock_(game_lock),
      state_(kUninitialized),
      hwnd_(nullptr),
      peek_message_ansi_(&Win32Window::PeekMessageDetourA, this),
      peek_message_unicode_(&Win32Window::PeekMessageDetourW, this) {}

Win32Window::~Win32Window() {}

bool Win32Window::Initialize() {
  CHECK_EQ(state_, kUninitialized);
  state_ = kInitializing;

  peek_message_ansi_.Install(L"user32.dll", "PeekMessageA");
  peek_message_unicode_.Install(L"user32.dll", "PeekMessageW");
  return true;
}

void Win32Window::Shutdown() {
  PIPE_LOG("[Window] Shutdown");
  if (state_ == kInitialized) {
    PIPE_LOG("[Window] Set shutdown state");
    state_ = kShutdown;
    PIPE_LOG("[Window] Wait for state change");
    state_.wait(kShutdown);
  }
  PIPE_LOG("[Window] Uninstall detours");
  peek_message_ansi_.Uninstall();
  peek_message_unicode_.Uninstall();
  PIPE_LOG("[Window] Shutdown complete");
}

bool Win32Window::IsInitialized() {
  return state_ == kInitialized;
}

void Win32Window::SetActiveWindow(HWND hWnd) {
  if (hWnd != hwnd_) {
    PIPE_LOG("[Window] Setting active output window to {:p}", static_cast<void*>(hWnd));
    hwnd_ = hWnd;
  }
}

LRESULT Win32Window::WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
  PushInputEventsForMessage(Msg, wParam, lParam, hWnd);

  // Check nyx ImGui WantCapture flags to block game input
  // TODO: could be a map lookup: nyx_imgui_->WantBlockMessage(Msg)
  switch (Msg) {
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP:
    case WM_CHAR:
      if (nyx_imgui_->WantCaptureKeyboard()) {
        return 0;
      }
      break;

    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONDBLCLK:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONDBLCLK:
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_XBUTTONUP:
    case WM_MOUSEWHEEL:
    case WM_MOUSEHWHEEL:
      if (nyx_imgui_->WantCaptureMouse()) {
        return 0;
      }
      break;
  }

  return game_->OnMessage(hWnd, Msg, wParam, lParam);
}

BOOL Win32Window::PeekMessageDetour(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg) {
  BOOL message_available = 0;
  if (IsWindowUnicode(hWnd) && peek_message_unicode_.IsEnabled()) {
    message_available = peek_message_unicode_.CallOriginal(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
  } else if (peek_message_ansi_.IsEnabled()) {
    message_available = peek_message_ansi_.CallOriginal(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
  }

  switch (state_) {
    case kUninitialized:
      break;

    case kInitializing:
      if (!hwnd_) {
        break;
      }
      PIPE_LOG("[Window] Initialized");
      state_ = kInitialized;
      state_.notify_all();
      break;

    case kInitialized: {
      bool consume_message = false;
      if (message_available != 0 && lpMsg->hwnd == hwnd_) {
        consume_message = WndProc(lpMsg->hwnd, lpMsg->message, lpMsg->wParam, lpMsg->lParam) == 0;
        if (consume_message) {
          TranslateMessage(lpMsg);
        }
      }

      if (message_available == 0) {
        RECT rect;
        ::GetClientRect(hwnd_, &rect);
        nyx::ImGuiInputEvent ev;
        ev.type = nyx::ImGuiInputEvent::kDisplaySize;
        ev.display_size = {(float)(rect.right - rect.left), (float)(rect.bottom - rect.top)};
        nyx_imgui_->PushInputEvent(ev);

        // yield to for safe memory reads, pumping main-thread work while open
        if (game_lock_) {
          game_lock_->Open(dolos::DrainMainThreadQueue);
        }
      }
      if (consume_message) {
        return 0;
      }
    } break;

    case kShutdown:
      state_ = kUninitialized;
      state_.notify_all();
      break;
  }

  return message_available;
}

void Win32Window::PushInputEventsForMessage(UINT Msg, WPARAM wParam, LPARAM lParam, HWND hWnd) {
  nyx::ImGuiInputEvent ev;

  switch (Msg) {
    case WM_MOUSEMOVE: {
      ev.type = nyx::ImGuiInputEvent::kMousePos;
      ev.mouse_pos = {(float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam)};
      nyx_imgui_->PushInputEvent(ev);
      break;
    }

    case WM_MOUSELEAVE: {
      ev.type = nyx::ImGuiInputEvent::kMousePos;
      ev.mouse_pos = {-FLT_MAX, -FLT_MAX};
      nyx_imgui_->PushInputEvent(ev);
      break;
    }

    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONDBLCLK:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONDBLCLK: {
      int button = 0;
      if (Msg == WM_LBUTTONDOWN || Msg == WM_LBUTTONDBLCLK) button = 0;
      if (Msg == WM_RBUTTONDOWN || Msg == WM_RBUTTONDBLCLK) button = 1;
      if (Msg == WM_MBUTTONDOWN || Msg == WM_MBUTTONDBLCLK) button = 2;
      if (Msg == WM_XBUTTONDOWN || Msg == WM_XBUTTONDBLCLK) button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4;
      ev.type = nyx::ImGuiInputEvent::kMouseButton;
      ev.mouse_button = {button, true};
      nyx_imgui_->PushInputEvent(ev);
      break;
    }

    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_XBUTTONUP: {
      int button = 0;
      if (Msg == WM_LBUTTONUP) button = 0;
      if (Msg == WM_RBUTTONUP) button = 1;
      if (Msg == WM_MBUTTONUP) button = 2;
      if (Msg == WM_XBUTTONUP) button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4;
      ev.type = nyx::ImGuiInputEvent::kMouseButton;
      ev.mouse_button = {button, false};
      nyx_imgui_->PushInputEvent(ev);
      break;
    }

    case WM_MOUSEWHEEL: {
      ev.type = nyx::ImGuiInputEvent::kMouseWheel;
      ev.mouse_wheel = {0.0f, (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA};
      nyx_imgui_->PushInputEvent(ev);
      break;
    }

    case WM_MOUSEHWHEEL: {
      ev.type = nyx::ImGuiInputEvent::kMouseWheel;
      ev.mouse_wheel = {-(float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA, 0.0f};
      nyx_imgui_->PushInputEvent(ev);
      break;
    }

    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP: {
      if (wParam < 256) {
        const bool is_key_down = (Msg == WM_KEYDOWN || Msg == WM_SYSKEYDOWN);

        // Push modifier state
        nyx::ImGuiInputEvent mod;
        mod.type = nyx::ImGuiInputEvent::kKey;
        mod.key = {ImGuiMod_Ctrl, (::GetKeyState(VK_CONTROL) & 0x8000) != 0};
        nyx_imgui_->PushInputEvent(mod);
        mod.key = {ImGuiMod_Shift, (::GetKeyState(VK_SHIFT) & 0x8000) != 0};
        nyx_imgui_->PushInputEvent(mod);
        mod.key = {ImGuiMod_Alt, (::GetKeyState(VK_MENU) & 0x8000) != 0};
        nyx_imgui_->PushInputEvent(mod);
        mod.key = {ImGuiMod_Super, (::GetKeyState(VK_APPS) & 0x8000) != 0};
        nyx_imgui_->PushInputEvent(mod);

        int vk = (int)wParam;
        if ((wParam == VK_RETURN) && (HIWORD(lParam) & KF_EXTENDED)) vk = IM_VK_KEYPAD_ENTER;
        const ImGuiKey key = VirtualKeyToImGuiKey(vk);

        if (key == ImGuiKey_PrintScreen && !is_key_down) {
          ev.type = nyx::ImGuiInputEvent::kKey;
          ev.key = {key, true};
          nyx_imgui_->PushInputEvent(ev);
        }

        if (key == ImGuiKey_Home) {
          nyx::Restart();
        }

        if (key == ImGuiKey_End) {
          nyx::Shutdown();
        }

        if (key != ImGuiKey_None) {
          ev.type = nyx::ImGuiInputEvent::kKey;
          ev.key = {key, is_key_down};
          nyx_imgui_->PushInputEvent(ev);
        }
      }
      break;
    }

    case WM_SETFOCUS:
    case WM_KILLFOCUS: {
      ev.type = nyx::ImGuiInputEvent::kFocus;
      ev.focus = {Msg == WM_SETFOCUS};
      nyx_imgui_->PushInputEvent(ev);
      break;
    }

    case WM_CHAR: {
      if (::IsWindowUnicode(hWnd)) {
        if (wParam > 0 && wParam < 0x10000) {
          ev.type = nyx::ImGuiInputEvent::kCharUTF16;
          ev.char_utf16 = {(unsigned short)wParam};
          nyx_imgui_->PushInputEvent(ev);
        }
      } else {
        wchar_t wch = 0;
        ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (char*)&wParam, 1, &wch, 1);
        ev.type = nyx::ImGuiInputEvent::kChar;
        ev.char_event = {(unsigned int)wch};
        nyx_imgui_->PushInputEvent(ev);
      }
      break;
    }
  }
}

BOOL Win32Window::PeekMessageDetourA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg) {
  return PeekMessageDetour(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
}

BOOL Win32Window::PeekMessageDetourW(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg) {
  return PeekMessageDetour(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
}

}  // namespace dolos
