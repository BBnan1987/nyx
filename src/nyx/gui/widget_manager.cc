#include "nyx/gui/widget_manager.h"

namespace nyx {

WidgetManager::~WidgetManager() {}

void WidgetManager::AddRoot(Widget* widget) {
  roots_.push_back(widget);
}

void WidgetManager::RemoveRoot(Widget* widget) {
  auto it = std::find(roots_.begin(), roots_.end(), widget);
  if (it != roots_.end()) {
    roots_.erase(it);
  }
}

void WidgetManager::UpdateAll() const {
  auto snapshot = roots_;
  for (Widget* root : snapshot) {
    root->Update();
  }
}

void WidgetManager::RenderAll() const {
  if (background_canvas_) {
    background_canvas_->Render(ImGui::GetBackgroundDrawList());
  }

  auto snapshot = roots_;
  for (Widget* root : snapshot) {
    if (root->visible()) {
      root->Render();
    }
  }

  if (foreground_canvas_) {
    foreground_canvas_->Render(ImGui::GetForegroundDrawList());
  }
}

}  // namespace nyx
