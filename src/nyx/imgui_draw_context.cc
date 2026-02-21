#include "nyx/imgui_draw_context.h"

#include "nyx/nyx_imgui.h"

namespace nyx {

ImGuiDrawContext::ImGuiDrawContext(NyxImGui* nyx_imgui) : nyx_imgui_(nyx_imgui) {
  ctx_ = ImGui::CreateContext(nyx_imgui_->font_atlas());

  ImGui::SetCurrentContext(ctx_);
  ImGuiIO& io = ImGui::GetIO();
  io.IniFilename = nullptr;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
  io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
  ImGui::StyleColorsDark();
  ImGui::GetStyle().FrameBorderSize = 0.0f;
}

ImGuiDrawContext::~ImGuiDrawContext() {
  if (ctx_) {
    ImGui::DestroyContext(ctx_);
    ctx_ = nullptr;
  }
}

void ImGuiDrawContext::BeginFrame() {
  if (frame_active_) {
    return;
  }

  GImGui = ctx_;

  auto events = nyx_imgui_->DrainInputEvents();
  ImGuiIO& io = ImGui::GetIO();
  ImGuiInputEvent::ReplayTo(io, events.data(), static_cast<int>(events.size()));

  auto now = std::chrono::steady_clock::now();
  float dt = std::chrono::duration<float>(now - last_frame_time_).count();
  last_frame_time_ = now;
  io.DeltaTime = (dt > 0.0f && dt < 0.1f) ? dt : (1.0f / 60.0f);

  ImGui::NewFrame();

  nyx_imgui_->SetWantCapture(io.WantCaptureMouse, io.WantCaptureKeyboard);

  frame_active_ = true;
}

void ImGuiDrawContext::EndFrame() {
  if (!frame_active_) {
    return;
  }

  ImGui::Render();

  ImDrawData* draw_data = ImGui::GetDrawData();
  if (draw_data && draw_data->Valid && draw_data->CmdListsCount > 0) {
    ImDrawList* bg_list = ImGui::GetBackgroundDrawList();

    // Split draw lists: background vs foreground
    ImVector<ImDrawList*> fg_lists;
    ImDrawList* bg_found = nullptr;

    for (int i = 0; i < draw_data->CmdListsCount; ++i) {
      if (draw_data->CmdLists[i] == bg_list) {
        bg_found = draw_data->CmdLists[i];
      } else {
        fg_lists.push_back(draw_data->CmdLists[i]);
      }
    }

    if (fg_lists.Size > 0) {
      nyx_imgui_->foreground()->Submit(
          fg_lists.Data, fg_lists.Size, draw_data->DisplayPos, draw_data->DisplaySize, draw_data->FramebufferScale);
    }

    if (bg_found && bg_found->CmdBuffer.Size > 0) {
      nyx_imgui_->background()->Submit(
          &bg_found, 1, draw_data->DisplayPos, draw_data->DisplaySize, draw_data->FramebufferScale);
    }
  } else {
    nyx_imgui_->ClearDrawData();
  }

  frame_active_ = false;
}

}  // namespace nyx
