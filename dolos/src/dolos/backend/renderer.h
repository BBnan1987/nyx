#pragma once

#include <Windows.h>
#include <atomic>

namespace dolos {

class Renderer {
 public:
  Renderer() noexcept = default;
  virtual ~Renderer() noexcept = default;

  virtual bool Initialize() = 0;
  virtual void Shutdown() = 0;

 protected:
  enum RendererState {
    kUninitialized,
    kInitializing,
    kRecreate,
    kInitialized,
    kShutdown,
  };
  std::atomic<RendererState> state_;
};

class TemporaryWindow {
 public:
  TemporaryWindow();
  ~TemporaryWindow();

  HWND hwnd;
};

}  // namespace dolos
