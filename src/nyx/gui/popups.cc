#include "nyx/gui/popups.h"

namespace nyx {

PopupWidget::PopupWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& id)
    : Widget(realm, object), id_(id) {}

void PopupWidget::Render() {
  if (should_open_) {
    ImGui::OpenPopup(id_.c_str());
    should_open_ = false;
  }
  if (ImGui::BeginPopup(id_.c_str())) {
    RenderChildren();
    if (should_close_) {
      ImGui::CloseCurrentPopup();
      should_close_ = false;
    }
    ImGui::EndPopup();
  }
}

ModalWidget::ModalWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& title)
    : Widget(realm, object), title_(title) {}

void ModalWidget::Render() {
  if (should_open_) {
    ImGui::OpenPopup(title_.c_str());
    should_open_ = false;
  }
  is_open_ = ImGui::BeginPopupModal(title_.c_str(), nullptr);
  if (is_open_) {
    RenderChildren();
    if (should_close_) {
      ImGui::CloseCurrentPopup();
      should_close_ = false;
    }
    ImGui::EndPopup();
  }
}

}  // namespace nyx
