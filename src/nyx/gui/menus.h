#pragma once

#include "nyx/gui/widget.h"

/** Implements:
 * + ImGui::BeginMenuBar
 * + ImGui::EndMenuBar
 * + ImGui::BeginMainMenuBar
 * + ImGui::EndMainMenuBar
 * + ImGui::BeginMenu
 * + ImGui::EndMenu
 * + ImGui::MenuItem
 */

namespace nyx {

class MainMenuBarWidget : public Widget {
 public:
  using Widget::Widget;
  void Render() override;
  bool IsContainer() const override { return true; }
};

class MenuBarWidget : public Widget {
 public:
  using Widget::Widget;
  void Render() override;
  bool IsContainer() const override { return true; }
};

class MenuWidget : public Widget {
 public:
  MenuWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label);
  void Render() override;
  bool IsContainer() const override { return true; }
  const std::string& label() const { return label_; }
  void set_label(const std::string& l) { label_ = l; }

 private:
  std::string label_;
};

class MenuItemWidget : public Widget {
 public:
  MenuItemWidget(
      Realm* realm, v8::Local<v8::Object> object, const std::string& label, const std::string& shortcut, bool selected);
  void Render() override;
  const std::string& label() const { return label_; }
  void set_label(const std::string& l) { label_ = l; }
  bool selected() const { return selected_; }
  void set_selected(bool s) { selected_ = s; }
  bool clicked() const { return clicked_; }

 private:
  std::string label_;
  std::string shortcut_;
  bool selected_;
  bool clicked_ = false;
};

}  // namespace nyx
