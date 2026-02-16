#pragma once

#include "nyx/base_object.h"
#include "nyx/imgui_v8.h"
#include "nyx/util.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

/** Needs implementations:
 * - ImGui::SetItemDefaultFocus
 * - ImGui::SetKeyboardFocusHere
 * - ImGui::SetNextItemAllowOverlap
 * - ImGui::IsItemHovered
 * - ImGui::IsItemActive
 * - ImGui::IsItemFocused
 * - ImGui::IsItemClicked
 * - ImGui::IsItemVisible
 * - ImGui::IsItemEdited
 * - ImGui::IsItemActivated
 * - ImGui::IsItemDeactivated
 * - ImGui::IsItemDeactivatedAfterEdit
 * - ImGui::IsItemToggledOpen
 * - ImGui::IsAnyItemHovered
 * - ImGui::IsAnyItemActive
 * - ImGui::IsAnyItemFocused
 * - ImGui::GetItemID
 * - ImGui::GetItemRectMin
 * - ImGui::GetItemRectMax
 * - ImGui::GetItemRectSize
 * - ImGui::SetItemAllowOverlap
 * - ImGui::GetMainViewport
 * - ImGui::GetBackgroundDrawList
 * - ImGui::GetForegroundDrawList
 * - ImGui::IsRectVisible
 * - ImGui::GetTime?
 * - ImGui::GetFrameCount
 * - ImGui::GetStyleColorName
 * - ImGui::BeginChildFrame
 * - ImGui::EndChildFrame
 * - ImGui::CalcTextSize
 * - ImGui::ColorConvertU32ToFloat4
 * - ImGui::ColorConvertFloat4ToU32
 * - ImGui::ColorConvertRGBToHSV
 * - ImGui::ColorConvertHSVToRGB
 * - ImGui::IsKeyDown
 * - ImGui::IsKeyPressed
 * - ImGui::IsKeyReleased
 * - ImGui::GetKeyPressedAmount
 * - ImGui::GetKeyName
 * - ImGui::SetNextFrameWantCaptureKeyboard
 * - ImGui::IsMouseDown
 * - ImGui::IsMouseClicked
 * - ImGui::IsMouseReleased
 * - ImGui::IsMouseDoubleClicked
 * - ImGui::GetMouseClickedCount
 * - ImGui::IsMouseHoveringRect
 * - ImGui::IsMousePosValid
 * - ImGui::IsAnyMouseDown
 * - ImGui::GetMousePos
 * - ImGui::GetMousePosOnOpeningCurrentPopup
 * - ImGui::IsMouseDragging
 * - ImGui::GetMouseDragDelta
 * - ImGui::ResetMouseDragDelta
 * - ImGui::GetMouseCursor
 * - ImGui::SetMouseCursor
 * - ImGui::GetClipboardText
 * - ImGui::SetClipboardText
 * - ImGui::SetNextItemWidth
 * - ImGui::CalcItemWidth
 * - ImGui::GetFont
 * - ImGui::GetFontSize
 * - ImGui::GetFontTexUvWhitePixel
 * - ImGui::GetColorU32
 * - ImGui::GetStyleColorVec4
 * - ImGui::GetID
 */

namespace nyx {

class Environment;
class WidgetManager;

class Widget : public BaseObject {
 public:
  Widget(Realm* realm, v8::Local<v8::Object> object);
  ~Widget() override;

  virtual void Render() = 0;
  virtual void Update() {
    EmitEvent("update");
    UpdateChildren();
  }

  virtual bool IsContainer() const { return false; }

  Widget* parent() const { return parent_; }
  const std::vector<Widget*>& children() const { return children_; }
  void AddChild(Widget* child);
  void RemoveChild(Widget* child);
  void ClearChildren();

  bool visible() const { return visible_; }
  void set_visible(bool v) { visible_ = v; }

  void On(const std::string& event, v8::Local<v8::Function> callback);
  void Off(const std::string& event, v8::Local<v8::Function> callback);

 protected:
  void UpdateChildren();
  void RenderChildren();
  void EmitEvent(const std::string& event);
  void EmitEvent(const std::string& event, v8::Local<v8::Value> arg);

  Widget* parent_ = nullptr;
  std::vector<Widget*> children_;
  bool visible_ = true;
  std::unordered_map<std::string, std::vector<v8::Global<v8::Function>>> event_handlers_;
};

class ChildWidget : public Widget {
 public:
  ChildWidget(Realm* realm,
              v8::Local<v8::Object> object,
              const std::string& id,
              float width,
              float height,
              ImGuiChildFlags child_flags,
              ImGuiWindowFlags window_flags);
  void Render() override;
  bool IsContainer() const override { return true; }

 private:
  std::string id_;
  float width_, height_;
  ImGuiChildFlags child_flags_;
  ImGuiWindowFlags window_flags_;
};

// To use ImGui::ShowDemoWindow(...) for testing
class DemoWindowWidget : public Widget {
 public:
  using Widget::Widget;
  void Render() override;
};

}  // namespace nyx
