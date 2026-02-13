#pragma once

#include <imgui.h>
#include <mutex>

namespace nyx {

class ImGuiDrawDataStore {
 public:
  ~ImGuiDrawDataStore();

  void Submit(ImDrawList* const* lists,
              int count,
              const ImVec2& display_pos,
              const ImVec2& display_size,
              const ImVec2& framebuffer_scale);
  void Clear();

  ImDrawData* Acquire();
  void Release();

 private:
  std::mutex mutex_;
  ImDrawData draw_data_;
  bool has_data_ = false;
};

}  // namespace nyx
