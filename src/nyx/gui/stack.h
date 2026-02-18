#pragma once

#include "nyx/gui/widget.h"

#include <map>

/** Implements:
 * + ImGui::PushID
 * + ImGui::PopID
 * + ImGui::PushClipRect
 * + ImGui::PopClipRect
 * - ImGui::PushFont
 * - ImGui::PopFont
 * + ImGui::PushStyleColor
 * + ImGui::PopStyleColor
 * + ImGui::PushStyleVar
 * + ImGui::PopStyleVar
 * + ImGui::PushTabStop
 * + ImGui::PopTabStop
 * + ImGui::PushButtonRepeat
 * + ImGui::PopButtonRepeat
 * + ImGui::PushItemWidth
 * + ImGui::PopItemWidth
 * + ImGui::PushTextWrapPos
 * + ImGui::PopTextWrapPos
 */

namespace nyx {

class StackWidget : public Widget {
 public:
  using Widget::Widget;

  static void Initialize(IsolateData* isolate_data, v8::Local<v8::ObjectTemplate> target);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void GetID(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void SetID(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void GetClipRect(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void SetClipRect(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void GetColors(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void SetColor(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void GetVars(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void SetVar(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void GetTabStop(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void SetTabStop(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void GetButtonRepeat(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void SetButtonRepeat(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void GetItemWidth(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void SetItemWidth(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void GetTextWrap(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void SetTextWrap(const v8::FunctionCallbackInfo<v8::Value>& args);

  void Render() override;
  bool IsContainer() const override { return true; }

  struct ClipRectData {
    ImVec2 min;
    ImVec2 max;
    bool intersect_with_current_clip_rect;
  };

  // [Push|Pop]ID
  bool id() const { return id_; }
  void set_id(uint32_t id) { id_ = id; }

  // [Push|Pop]ClipRect
  const ClipRectData& clip_rect() const { return clip_rect_data_; }
  void set_clip_rect(ClipRectData data) { clip_rect_data_ = data; }
  
  // [Push|Pop]Font
  // NYI

  // [Push|Pop]StyleColor
  const std::map<ImGuiCol, ImVec4>& colors() const { return colors_; }
  void set_color(ImGuiCol idx, const ImVec4& col) { colors_[idx] = col; }
  
  // [Push|Pop]StyleVar
  const std::map<ImGuiStyleVar, ImVec2>& vars() const { return vars_; }
  void set_var(ImGuiStyleVar idx, const ImVec2& var) { vars_[idx] = var; }
  
  // [Push|Pop]TabStop
  bool tab_stop() const { return tab_stop_; }
  void set_tab_stop(bool tab_stop) { tab_stop_ = tab_stop; }
  
  // [Push|Pop]ButtonRepeat
  bool button_repeat() const { return button_repeat_; }
  void set_button_repeat(bool button_repeat) { button_repeat_ = button_repeat; }
  
  // [Push|Pop]ItemWidth
  float item_width() const { return item_width_; }
  void set_item_width(float item_width) { item_width_ = item_width; }
  
  // [Push|Pop]TextWrapPos
  float text_wrap() const { return text_wrap_; }
  void set_text_wrap(float text_wrap) { text_wrap_ = text_wrap; }

 private:
  int32_t id_ = -1;
  ClipRectData clip_rect_data_{{-1.0f, -1.0f}, {-1.0f, -1.0f}, false};
  std::map<ImGuiCol, ImVec4> colors_;
  std::map<ImGuiStyleVar, ImVec2> vars_;
  bool tab_stop_ = false;
  bool button_repeat_ = false;
  float item_width_ = 0.0f;
  float text_wrap_ = 0.0f;
};

}  // namespace nyx
