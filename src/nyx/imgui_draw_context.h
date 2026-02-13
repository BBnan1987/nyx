#pragma once

#include <imgui.h>
#include <chrono>

namespace nyx {

class NyxImGui;

class ImGuiDrawContext {
 public:
  explicit ImGuiDrawContext(NyxImGui* nyx_imgui);
  ~ImGuiDrawContext();

  ImGuiDrawContext(const ImGuiDrawContext&) = delete;
  ImGuiDrawContext& operator=(const ImGuiDrawContext&) = delete;

  void BeginFrame();
  void EndFrame();

  ImGuiContext* context() const { return ctx_; }
  bool frame_active() const { return frame_active_; }

 private:
  ImGuiContext* ctx_;
  NyxImGui* nyx_imgui_;  // shared, not owned
  bool frame_active_ = false;
  std::chrono::steady_clock::time_point last_frame_time_;
};

}  // namespace nyx
