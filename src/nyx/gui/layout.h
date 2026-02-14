#pragma once

#include "nyx/gui/widget.h"

/** Implements:
 * + ImGui::Separator
 * + ImGui::SameLine
 * - ImGui::NewLine
 * + ImGui::Spacing
 * + ImGui::Dummy
 * + ImGui::Indent
 * + ImGui::Unindent
 * + ImGui::BeginGroup
 * + ImGui::EndGroup
 * + ImGui::BeginDisabled
 * + ImGui::EndDisabled
 * - ImGui::GetCursorPos
 * - ImGui::GetCursorPosX
 * - ImGui::GetCursorPosY
 * - ImGui::SetCursorPos
 * - ImGui::SetCursorPosX
 * - ImGui::SetCursorPosY
 * - ImGui::GetCursorStartPos
 * - ImGui::GetCursorScreenPos
 * - ImGui::SetCursorScreenPos
 * - ImGui::AlignTextToFramePadding
 * - ImGui::GetTextLineHeight
 * - ImGui::GetTextLineHeightWithSpacing
 * - ImGui::GetFrameHeight
 * - ImGui::GetFrameHeightWithSpacing
 */

namespace nyx {

class SeparatorWidget : public Widget {
 public:
  using Widget::Widget;
  void Render() override;
};

class SpacingWidget : public Widget {
 public:
  using Widget::Widget;
  void Render() override;
};

class SameLineWidget : public Widget {
 public:
  SameLineWidget(Realm* realm, v8::Local<v8::Object> object, float offset, float spacing);

  void Render() override;

 private:
  float offset_;
  float spacing_;
};

class NewLineWidget : public Widget {
 public:
  using Widget::Widget;
  void Render() override;
};

class IndentWidget : public Widget {
 public:
  IndentWidget(Realm* realm, v8::Local<v8::Object> object, float width);
  void Render() override;
  bool IsContainer() const override { return true; }

 private:
  float width_;
};

class UnindentWidget : public Widget {
 public:
  UnindentWidget(Realm* realm, v8::Local<v8::Object> object, float width);
  void Render() override;
  bool IsContainer() const override { return true; }

 private:
  float width_;
};

class DummyWidget : public Widget {
 public:
  DummyWidget(Realm* realm, v8::Local<v8::Object> object, float width, float height);
  void Render() override;

 private:
  float width_, height_;
};

class GroupWidget : public Widget {
 public:
  using Widget::Widget;
  void Render() override;
  bool IsContainer() const override { return true; }
};

class DisabledWidget : public Widget {
 public:
  DisabledWidget(Realm* realm, v8::Local<v8::Object> object, bool disabled);
  void Render() override;
  bool IsContainer() const override { return true; }
  bool disabled() const { return disabled_; }
  void set_disabled(bool d) { disabled_ = d; }

 private:
  bool disabled_;
};

}  // namespace nyx
