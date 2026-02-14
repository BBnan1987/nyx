#include "nyx/gui/trees.h"

namespace nyx {

TreeNodeWidget::TreeNodeWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label)
    : Widget(realm, object), label_(label) {}

void TreeNodeWidget::Render() {
  open_ = ImGui::TreeNode(label_.c_str());
  if (open_) {
    RenderChildren();
    ImGui::TreePop();
  }
}

CollapsingHeaderWidget::CollapsingHeaderWidget(Realm* realm,
                                               v8::Local<v8::Object> object,
                                               const std::string& label,
                                               ImGuiTreeNodeFlags flags)
    : Widget(realm, object), label_(label), flags_(flags) {}

void CollapsingHeaderWidget::Render() {
  open_ = ImGui::CollapsingHeader(label_.c_str(), flags_);
  if (open_) {
    RenderChildren();
  }
}

}  // namespace nyx
