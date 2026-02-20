#pragma once

#include "nyx/gui/widget.h"

/** Implements:
 * + ImGui::BeginCombo
 * + ImGui::EndCombo
 * + ImGui::Combo
 */

namespace nyx {

  /** todo
  * 1. preview value format
  * 2. per-item options (flags, size)
  * 3. should Selectable be split into separate widget?
  *     a. easier to have per-item options
  *     b. can add other widgets between
  *     c. would need to pass along selected_ if that should be handled automatically
  */
class ComboWidget : public Widget {
 public:
  ComboWidget(Realm* realm,
              v8::Local<v8::Object> object,
              const std::string& label,
              std::vector<std::string> items,
              int selected,
              ImGuiComboFlags flags = ImGuiComboFlags_None);

  static void Initialize(IsolateData* isolate_data, v8::Local<v8::ObjectTemplate> target);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void FlagsGetter(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void FlagsSetter(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void SelectedGetter(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void SelectedSetter(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void ItemsGetter(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void ItemsSetter(const v8::FunctionCallbackInfo<v8::Value>& args);

  void Render() override;

 private:
  ImGuiComboFlags flags_;
  std::vector<std::string> items_;
  int selected_;
};

}  // namespace nyx
