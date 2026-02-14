#include "nyx/gui/widget_manager.h"

namespace nyx {

void WidgetManager::AddRoot(Widget* widget) {
  roots_.push_back(widget);
}

void WidgetManager::RemoveRoot(Widget* widget) {
  auto it = std::find(roots_.begin(), roots_.end(), widget);
  if (it != roots_.end()) {
    roots_.erase(it);
  }
}

void WidgetManager::RenderAll() {
  // Snapshot in case event handlers add/remove roots
  auto snapshot = roots_;
  for (Widget* root : snapshot) {
    if (root->visible()) {
      root->Render();
    }
  }
}

}  // namespace nyx
