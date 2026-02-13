#pragma once

#include "dolos/detour.h"

#include <Windows.h>
#include <atomic>
#include <functional>

namespace nyx {
class GameLock;
class NyxImGui;
}  // namespace nyx

namespace dolos {

class Game;

class Win32Window {
  enum WindowState {
    kUninitialized,
    kInitializing,
    kInitialized,
    kShutdown,
  };

 public:
  Win32Window(Game* game, nyx::NyxImGui* nyx_imgui, nyx::GameLock* game_lock);
  ~Win32Window();

  Win32Window(const Win32Window&) = delete;
  Win32Window& operator=(const Win32Window&) = delete;
  Win32Window(Win32Window&&) = delete;
  Win32Window& operator=(Win32Window&&) = delete;

  bool Initialize();
  void Shutdown();

  bool IsInitialized();
  void SetActiveWindow(HWND hWnd);

 private:
  Game* game_;
  nyx::NyxImGui* nyx_imgui_;
  nyx::GameLock* game_lock_;
  std::atomic<WindowState> state_;
  HWND hwnd_;

  LRESULT WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
  void PushInputEventsForMessage(UINT Msg, WPARAM wParam, LPARAM lParam, HWND hWnd);

  BOOL PeekMessageDetour(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);

  Detour<decltype(&PeekMessageA)> peek_message_ansi_;
  Detour<decltype(&PeekMessageW)> peek_message_unicode_;

  // Due to a bug in MinHook we have separate detours for both PeekMessage versions
  BOOL PeekMessageDetourA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);
  BOOL PeekMessageDetourW(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);
};

}  // namespace dolos
