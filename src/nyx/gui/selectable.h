#pragma once

#include "nyx/gui/widget.h"

/** Implements:
 * + ImGui::Selectable
 */

namespace nyx {

class SelectableWidget : public Widget {
 public:
  SelectableWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label, bool selected);

  static void Initialize(IsolateData* isolate_data, v8::Local<v8::ObjectTemplate> target);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void GetSelected(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void SetSelected(const v8::FunctionCallbackInfo<v8::Value>& args);

  void Render() override;
  bool selected() const { return selected_; }
  void set_selected(bool s) { selected_ = s; }

 private:
  bool selected_;
};

}  // namespace nyx
