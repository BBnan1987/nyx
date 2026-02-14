#include "nyx/gui/tabs.h"

namespace nyx {

TabBarWidget::TabBarWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& id)
    : Widget(realm, object), id_(id) {}

void TabBarWidget::Render() {
  if (ImGui::BeginTabBar(id_.c_str())) {
    RenderChildren();
    ImGui::EndTabBar();
  }
}

TabItemWidget::TabItemWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label)
    : Widget(realm, object), label_(label) {}

void TabItemWidget::Render() {
  selected_ = ImGui::BeginTabItem(label_.c_str());
  if (selected_) {
    RenderChildren();
    ImGui::EndTabItem();
  }
}

}  // namespace nyx
