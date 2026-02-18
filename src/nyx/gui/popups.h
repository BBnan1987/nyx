#pragma once

#include "nyx/gui/widget.h"

/** Implements:
 * + ImGui::BeginPopup
 * + ImGui::BeginPopupModal
 * + ImGui::EndPopup
 * + ImGui::OpenPopup
 * - ImGui::OpenPopupOnItemClick
 * + ImGui::CloseCurrentPopup
 * - ImGui::BeginPopupContextItem
 * - ImGui::BeginPopupContextWindow
 * - ImGui::BeginPopupContextVoid
 * - ImGui::IsPopupOpen
 */

namespace nyx {

class PopupWidget : public Widget {
 public:
  PopupWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& id);

  static void Initialize(IsolateData* isolate_data, v8::Local<v8::ObjectTemplate> target);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void Open(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Close(const v8::FunctionCallbackInfo<v8::Value>& args);

  void Render() override;
  bool IsContainer() const override { return true; }
  void DoOpen() { should_open_ = true; }
  void DoClose() { should_close_ = true; }

 private:
  std::string id_;
  bool should_open_ = false;
  bool should_close_ = false;
};

class ModalWidget : public Widget {
 public:
  ModalWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& title);

  static void Initialize(IsolateData* isolate_data, v8::Local<v8::ObjectTemplate> target);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void Open(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Close(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void IsOpen(const v8::FunctionCallbackInfo<v8::Value>& args);

  void Render() override;
  bool IsContainer() const override { return true; }
  void DoOpen() { should_open_ = true; }
  void DoClose() { should_close_ = true; }
  bool is_open() const { return is_open_; }

 private:
  std::string title_;
  bool should_open_ = false;
  bool should_close_ = false;
  bool is_open_ = false;
};

}  // namespace nyx
