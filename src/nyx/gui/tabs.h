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

  static void Initialize(IsolateData* isolate_data, v8::Local<v8::ObjectTemplate> target);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  void Render() override;
  bool IsContainer() const override { return true; }

 private:
  std::string id_;
};

class TabItemWidget : public Widget {
 public:
  TabItemWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label);

  static void Initialize(IsolateData* isolate_data, v8::Local<v8::ObjectTemplate> target);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void GetSelected(const v8::FunctionCallbackInfo<v8::Value>& args);

  void Render() override;
  bool IsContainer() const override { return true; }

  bool selected() const { return selected_; }

 private:
  bool selected_ = false;
};

}  // namespace nyx
