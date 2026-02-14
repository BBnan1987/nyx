#include "nyx/gui/tables.h"

namespace nyx {

TableWidget::TableWidget(
    Realm* realm, v8::Local<v8::Object> object, const std::string& id, int columns, ImGuiTableFlags flags)
    : Widget(realm, object), id_(id), columns_(columns), flags_(flags) {}

void TableWidget::AddColumn(const std::string& name, ImGuiTableColumnFlags flags, float width) {
  column_defs_.push_back({name, flags, width});
}

void TableWidget::Render() {
  if (ImGui::BeginTable(id_.c_str(), columns_, flags_)) {
    // Setup columns
    for (const auto& col : column_defs_) {
      ImGui::TableSetupColumn(col.name.c_str(), col.flags, col.width);
    }
    if (!column_defs_.empty()) {
      ImGui::TableHeadersRow();
    }
    RenderChildren();
    ImGui::EndTable();
  }
}

void TableRowWidget::Render() {
  ImGui::TableNextRow();
  // Each child occupies the next column
  auto snapshot = children_;
  for (size_t i = 0; i < snapshot.size(); i++) {
    ImGui::TableSetColumnIndex(static_cast<int>(i));
    if (snapshot[i]->visible()) {
      snapshot[i]->Render();
    }
  }
}

}  // namespace nyx
