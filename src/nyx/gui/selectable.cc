#include "nyx/gui/selectable.h"

namespace nyx {

using v8::HandleScope;
using v8::Isolate;

SelectableWidget::SelectableWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label, bool selected)
    : Widget(realm, object), label_(label), selected_(selected) {}

void SelectableWidget::Render() {
  bool old = selected_;
  ImGui::Selectable(label_.c_str(), &selected_);
  if (selected_ != old) {
    Isolate* iso = isolate();
    HandleScope scope(iso);
    EmitEvent("change", v8::Boolean::New(iso, selected_));
  }
}

}  // namespace nyx
