#pragma once

#include "nyx/gui/widget.h"

/** Implements:
 * + ImGui::BeginTabBar
 * + ImGui::EndTabBar
 * + ImGui::BeginTabItem
 * + ImGui::EndTabItem
 * - ImGui::TabItemButton
 * - ImGui::SetTabItemClosed
 */

namespace nyx {

class TabBarWidget : public Widget {
 public:
  TabBarWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& id);

  void Render() override;
  bool IsContainer() const override { return true; }

 private:
  std::string id_;
};

class TabItemWidget : public Widget {
 public:
  TabItemWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label);

  void Render() override;
  bool IsContainer() const override { return true; }

  const std::string& label() const { return label_; }
  void set_label(const std::string& l) { label_ = l; }
  bool selected() const { return selected_; }

 private:
  std::string label_;
  bool selected_ = false;
};

}  // namespace nyx
