#pragma once

#include "nyx/gui/widget.h"

/** Implements:
 * + ImGui::ColorEdit3
 * + ImGui::ColorEdit4
 * + ImGui::ColorPicker3
 * + ImGui::ColorPicker4
 * + ImGui::ColorButton
 * - ImGui::SetColorEditOptions should this be global or per-widget?
 */

namespace nyx {

class ColorWidget : public Widget {
 public:
  enum ColorWidgetType : int32_t { ColorEdit3, ColorEdit4, ColorPicker3, ColorPicker4, ColorButton };

  ColorWidget(Realm* realm,
              v8::Local<v8::Object> object,
              const std::string& label,
              ColorWidgetType type,
              ImGuiColorEditFlags flags,
              ImVec4 color,
              ImVec4 ref_color,
              ImVec2 size);

  static void Initialize(IsolateData* isolate_data, v8::Local<v8::ObjectTemplate> target);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void FlagsGetter(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void FlagsSetter(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void ColorGetter(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void ColorSetter(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void RefColorGetter(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void RefColorSetter(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void SizeGetter(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void SizeSetter(const v8::FunctionCallbackInfo<v8::Value>& args);

  void Render() override;

 private:
  ColorWidgetType type_;
  ImGuiColorEditFlags flags_;
  ImVec4 color_;
  ImVec4 ref_color_;  // used by ColorPicker4
  ImVec2 size_;       // used by ColorButton
};

}  // namespace nyx
