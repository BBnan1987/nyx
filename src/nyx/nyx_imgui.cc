#include "nyx/nyx_imgui.h"

// thread_local ImGui context pointer (declared in imconfig.h via #define GImGui)
thread_local ImGuiContext* NyxImGuiTLS = nullptr;

namespace nyx {

NyxImGui::NyxImGui() {
  font_atlas_ = IM_NEW(ImFontAtlas);
  font_atlas_->AddFontDefault();
  font_atlas_->Build();
}

NyxImGui::~NyxImGui() {
  if (font_atlas_) {
    IM_DELETE(font_atlas_);
    font_atlas_ = nullptr;
  }
}

void NyxImGui::EnsureGameContext() {
  if (GImGui != game_context_) {
    GImGui = game_context_;
  }
}

void NyxImGui::SetWantCapture(bool mouse, bool keyboard) {
  want_capture_mouse_.store(mouse, std::memory_order_relaxed);
  want_capture_keyboard_.store(keyboard, std::memory_order_relaxed);
}

bool NyxImGui::WantCaptureMouse() const {
  return want_capture_mouse_.load(std::memory_order_relaxed);
}

bool NyxImGui::WantCaptureKeyboard() const {
  return want_capture_keyboard_.load(std::memory_order_relaxed);
}

void NyxImGui::ClearDrawData() {
  foreground_.Clear();
  background_.Clear();
}

void NyxImGui::PushInputEvent(const ImGuiInputEvent& event) {
  std::scoped_lock lock(input_mutex_);
  input_queue_.push_back(event);
}

void NyxImGui::PushInputEvents(const ImGuiInputEvent* events, int count) {
  std::scoped_lock lock(input_mutex_);
  input_queue_.insert(input_queue_.end(), events, events + count);
}

std::vector<ImGuiInputEvent> NyxImGui::DrainInputEvents() {
  std::scoped_lock lock(input_mutex_);
  std::vector<ImGuiInputEvent> events;
  events.swap(input_queue_);
  return events;
}

bool NyxImGui::HasPendingInputEvents() {
  std::scoped_lock lock(input_mutex_);
  return !input_queue_.empty();
}

}  // namespace nyx
