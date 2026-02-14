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

  void Render() override;
  bool IsContainer() const override { return true; }

  const std::string& label() const { return label_; }
  void set_label(const std::string& l) { label_ = l; }
  bool open() const { return open_; }

 private:
  std::string label_;
  bool open_ = false;
};

class CollapsingHeaderWidget : public Widget {
 public:
  CollapsingHeaderWidget(Realm* realm,
                         v8::Local<v8::Object> object,
                         const std::string& label,
                         ImGuiTreeNodeFlags flags);

  void Render() override;
  bool IsContainer() const override { return true; }

  const std::string& label() const { return label_; }
  void set_label(const std::string& l) { label_ = l; }
  bool open() const { return open_; }

 private:
  std::string label_;
  ImGuiTreeNodeFlags flags_ = 0;
  bool open_ = false;
};

}  // namespace nyx
