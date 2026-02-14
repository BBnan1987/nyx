#pragma once

#include "nyx/gui/widget.h"

/** Implements:
 * + ImGui::BeginTable
 * + ImGui::EndTable
 * - ImGui::TableNextRow
 * - ImGui::TableNextColumn
 * - ImGui::TableSetColumnIndex
 * + ImGui::TableSetupColumn
 * - ImGui::TableSetupScrollFreeze
 * - ImGui::TableHeadersRow
 * - ImGui::TableHeader
 * - ImGui::TableGetSortSpecs
 * - ImGui::TableGetColumnCount
 * - ImGui::TableGetColumnIndex
 * - ImGui::TableGetRowIndex
 * - ImGui::TableGetColumnName
 * - ImGui::TableGetColumnFlags
 * - ImGui::TableGetColumnEnabled
 * - ImGui::TableSetBgColor
 */

namespace nyx {

class TableWidget : public Widget {
 public:
  TableWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& id, int columns, ImGuiTableFlags flags);
  void Render() override;
  bool IsContainer() const override { return true; }
  void AddColumn(const std::string& name, ImGuiTableColumnFlags flags, float width);

 private:
  std::string id_;
  int columns_;
  ImGuiTableFlags flags_;
  struct ColumnDef {
    std::string name;
    ImGuiTableColumnFlags flags;
    float width;
  };
  std::vector<ColumnDef> column_defs_;
};

class TableRowWidget : public Widget {
 public:
  using Widget::Widget;
  void Render() override;
  bool IsContainer() const override { return true; }
};

}  // namespace nyx
