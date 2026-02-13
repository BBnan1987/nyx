#include "nyx/imgui_draw_data_store.h"

namespace nyx {

ImGuiDrawDataStore::~ImGuiDrawDataStore() {
  draw_data_.CmdLists.clear_delete();
  draw_data_.Clear();
}

void ImGuiDrawDataStore::Submit(ImDrawList* const* lists,
                                int count,
                                const ImVec2& display_pos,
                                const ImVec2& display_size,
                                const ImVec2& framebuffer_scale) {
  std::scoped_lock lock(mutex_);

  draw_data_.CmdLists.clear_delete();
  draw_data_.Clear();

  if (!lists || count == 0) {
    has_data_ = false;
    return;
  }

  int total_vtx = 0;
  int total_idx = 0;
  draw_data_.CmdLists.resize(count);
  for (int i = 0; i < count; ++i) {
    draw_data_.CmdLists[i] = lists[i]->CloneOutput();
    total_vtx += lists[i]->VtxBuffer.Size;
    total_idx += lists[i]->IdxBuffer.Size;
  }

  draw_data_.TotalVtxCount = total_vtx;
  draw_data_.TotalIdxCount = total_idx;
  draw_data_.CmdListsCount = count;
  draw_data_.DisplayPos = display_pos;
  draw_data_.DisplaySize = display_size;
  draw_data_.FramebufferScale = framebuffer_scale;
  draw_data_.Valid = true;
  has_data_ = true;
}

void ImGuiDrawDataStore::Clear() {
  std::scoped_lock lock(mutex_);
  draw_data_.CmdLists.clear_delete();
  draw_data_.Clear();
  has_data_ = false;
}

ImDrawData* ImGuiDrawDataStore::Acquire() {
  mutex_.lock();
  if (!has_data_) {
    mutex_.unlock();
    return nullptr;
  }
  return &draw_data_;
}

void ImGuiDrawDataStore::Release() {
  mutex_.unlock();
}

}  // namespace nyx
