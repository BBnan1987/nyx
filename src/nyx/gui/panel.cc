#include "nyx/gui/panel.h"

namespace nyx {

PanelWidget::PanelWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& title, ImGuiWindowFlags flags)
    : Widget(realm, object), title_(title), flags_(flags) {}

void PanelWidget::Render() {
  if (!open_) return;

  bool was_open = open_;
  panel_visible_ = ImGui::Begin(title_.c_str(), &open_, flags_);
  if (panel_visible_) {
    RenderChildren();
  }
  ImGui::End();

  if (was_open && !open_) {
    EmitEvent("close");
  }
}

}  // namespace nyx
