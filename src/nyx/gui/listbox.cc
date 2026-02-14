#include "nyx/gui/listbox.h"

namespace nyx {

using v8::HandleScope;
using v8::Isolate;

ListBoxWidget::ListBoxWidget(Realm* realm,
                             v8::Local<v8::Object> object,
                             const std::string& label,
                             std::vector<std::string> items,
                             int selected,
                             int height_items)
    : Widget(realm, object),
      label_(label),
      items_(std::move(items)),
      selected_(selected),
      height_items_(height_items) {}

void ListBoxWidget::Render() {
  // Build const char* array for ImGui
  std::vector<const char*> c_items;
  c_items.reserve(items_.size());
  for (const auto& s : items_) c_items.push_back(s.c_str());

  int old = selected_;
  ImGui::ListBox(label_.c_str(), &selected_, c_items.data(), static_cast<int>(c_items.size()), height_items_);
  if (selected_ != old) {
    Isolate* iso = isolate();
    HandleScope scope(iso);
    EmitEvent("change", v8::Integer::New(iso, selected_));
  }
}

}  // namespace nyx
