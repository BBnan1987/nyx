#include "nyx/gui/combo.h"

namespace nyx {

using v8::HandleScope;
using v8::Isolate;

ComboWidget::ComboWidget(
    Realm* realm, v8::Local<v8::Object> object, const std::string& label, std::vector<std::string> items, int selected)
    : Widget(realm, object), label_(label), items_(std::move(items)), selected_(selected) {}

void ComboWidget::Render() {
  const char* preview =
      (selected_ >= 0 && selected_ < static_cast<int>(items_.size())) ? items_[selected_].c_str() : "";
  if (ImGui::BeginCombo(label_.c_str(), preview)) {
    for (int i = 0; i < static_cast<int>(items_.size()); i++) {
      bool is_selected = (i == selected_);
      if (ImGui::Selectable(items_[i].c_str(), is_selected)) {
        if (selected_ != i) {
          selected_ = i;
          Isolate* iso = isolate();
          HandleScope scope(iso);
          EmitEvent("change", v8::Integer::New(iso, selected_));
        }
      }
      if (is_selected) ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }
}

}  // namespace nyx
