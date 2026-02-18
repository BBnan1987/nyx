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

  static void Initialize(IsolateData* isolate_data, v8::Local<v8::ObjectTemplate> target);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  void Render() override;
  bool IsContainer() const override { return true; }
};

class MenuBarWidget : public Widget {
 public:
  using Widget::Widget;

  static void Initialize(IsolateData* isolate_data, v8::Local<v8::ObjectTemplate> target);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  void Render() override;
  bool IsContainer() const override { return true; }
};

class MenuWidget : public Widget {
 public:
  MenuWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label);

  static void Initialize(IsolateData* isolate_data, v8::Local<v8::ObjectTemplate> target);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  void Render() override;
  bool IsContainer() const override { return true; }

 private:
};

class MenuItemWidget : public Widget {
 public:
  MenuItemWidget(
      Realm* realm, v8::Local<v8::Object> object, const std::string& label, const std::string& shortcut, bool selected);

  static void Initialize(IsolateData* isolate_data, v8::Local<v8::ObjectTemplate> target);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void GetClicked(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void GetSelected(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void SetSelected(const v8::FunctionCallbackInfo<v8::Value>& args);

  void Render() override;
  bool selected() const { return selected_; }
  void set_selected(bool s) { selected_ = s; }
  bool clicked() const { return clicked_; }

 private:
  std::string shortcut_;
  bool selected_;
  bool clicked_ = false;
};

}  // namespace nyx
