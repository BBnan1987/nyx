#include "nyx/gui/menus.h"

namespace nyx {

using v8::HandleScope;
using v8::Isolate;

void MainMenuBarWidget::Render() {
  if (ImGui::BeginMainMenuBar()) {
    RenderChildren();
    ImGui::EndMainMenuBar();
  }
}

void MenuBarWidget::Render() {
  if (ImGui::BeginMenuBar()) {
    RenderChildren();
    ImGui::EndMenuBar();
  }
}

MenuWidget::MenuWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label)
    : Widget(realm, object), label_(label) {}

void MenuWidget::Render() {
  if (ImGui::BeginMenu(label_.c_str())) {
    RenderChildren();
    ImGui::EndMenu();
  }
}

MenuItemWidget::MenuItemWidget(
    Realm* realm, v8::Local<v8::Object> object, const std::string& label, const std::string& shortcut, bool selected)
    : Widget(realm, object), label_(label), shortcut_(shortcut), selected_(selected) {}

void MenuItemWidget::Render() {
  bool old_selected = selected_;
  clicked_ = ImGui::MenuItem(label_.c_str(), shortcut_.empty() ? nullptr : shortcut_.c_str(), &selected_);
  if (clicked_) {
    EmitEvent("click");
  }
  if (selected_ != old_selected) {
    Isolate* iso = isolate();
    HandleScope scope(iso);
    EmitEvent("change", v8::Boolean::New(iso, selected_));
  }
}

}  // namespace nyx
