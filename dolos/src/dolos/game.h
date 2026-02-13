#pragma once

#include <Windows.h>
#include <filesystem>

namespace dolos {

class Game {
 public:
  // implemented by consumers.
  static Game* Create();

  virtual ~Game() = default;

  virtual bool OnInitialize() = 0;
  virtual void OnShutdown() = 0;

  virtual LRESULT OnMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) { return 1; }

 protected:
  std::filesystem::path cwd_;
};

}  // namespace dolos
