#pragma once

#include "nyx/gui/widget.h"

/** Implements:
 * + ImGui::Selectable
 */

namespace nyx {

class SelectableWidget : public Widget {
 public:
  SelectableWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label, bool selected);
  void Render() override;
  const std::string& label() const { return label_; }
  void set_label(const std::string& l) { label_ = l; }
  bool selected() const { return selected_; }
  void set_selected(bool s) { selected_ = s; }

 private:
  std::string label_;
  bool selected_;
};

}  // namespace nyx
