#pragma once

#include "nyx/gui/widget.h"

/** Implements:
 * + ImGui::TreeNode
 * - ImGui::TreePush
 * - ImGui::TreePop
 * - ImGui::GetTreeNodeToLabelSpacing
 * + ImGui::CollapsingHeader
 * - ImGui::SetNextItemOpen
 */

namespace nyx {

class TreeNodeWidget : public Widget {
 public:
  TreeNodeWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label);

  static void Initialize(IsolateData* isolate_data, v8::Local<v8::ObjectTemplate> target);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void GetOpen(const v8::FunctionCallbackInfo<v8::Value>& args);

  void Render() override;
  bool IsContainer() const override { return true; }

  bool open() const { return open_; }

 private:
  bool open_ = false;
};

class CollapsingHeaderWidget : public Widget {
 public:
  CollapsingHeaderWidget(Realm* realm,
                         v8::Local<v8::Object> object,
                         const std::string& label,
                         ImGuiTreeNodeFlags flags);

  static void Initialize(IsolateData* isolate_data, v8::Local<v8::ObjectTemplate> target);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void GetOpen(const v8::FunctionCallbackInfo<v8::Value>& args);

  void Render() override;
  bool IsContainer() const override { return true; }

  bool open() const { return open_; }

 private:
  ImGuiTreeNodeFlags flags_ = 0;
  bool open_ = false;
};

}  // namespace nyx
