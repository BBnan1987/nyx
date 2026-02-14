#pragma once

#include "nyx/gui/widget.h"

/** Implements:
 * + ImGui::BeginCombo
 * + ImGui::EndCombo
 * + ImGui::Combo
 */

namespace nyx {

class ComboWidget : public Widget {
 public:
  ComboWidget(Realm* realm,
              v8::Local<v8::Object> object,
              const std::string& label,
              std::vector<std::string> items,
              int selected);
  void Render() override;
  const std::string& label() const { return label_; }
  void set_label(const std::string& l) { label_ = l; }
  int selected() const { return selected_; }
  void set_selected(int s) { selected_ = s; }
  const std::vector<std::string>& items() const { return items_; }
  void set_items(std::vector<std::string> items) { items_ = std::move(items); }

 private:
  std::string label_;
  std::vector<std::string> items_;
  int selected_;
};

}  // namespace nyx
