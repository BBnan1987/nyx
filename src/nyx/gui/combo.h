#pragma once

#include "nyx/gui/widget.h"

/** Implements:
 * + ImGui::BeginCombo
 * + ImGui::EndCombo
 * + ImGui::Combo
 */

namespace nyx {

class ComboWidget : public Widget {
 public:
  ComboWidget(Realm* realm,
              v8::Local<v8::Object> object,
              const std::string& label,
              std::vector<std::string> items,
              int selected);

  static void Initialize(IsolateData* isolate_data, v8::Local<v8::ObjectTemplate> target);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void SelectedGetter(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void SelectedSetter(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void SetItems(const v8::FunctionCallbackInfo<v8::Value>& args);

  void Render() override;

  int selected() const { return selected_; }
  void set_selected(int s) { selected_ = s; }
  const std::vector<std::string>& items() const { return items_; }
  void set_items(std::vector<std::string> items) { items_ = std::move(items); }

 private:
  std::vector<std::string> items_;
  int selected_;
};

}  // namespace nyx
