#pragma once

#include "nyx/gui/widget.h"

/** Implements:
 * + ImGui::InputText
 * + ImGui::InputTextMultiline
 * - ImGui::InputTextWithHint
 * + ImGui::InputFloat
 * + ImGui::InputInt
 */

namespace nyx {

class InputTextWidget : public Widget {
 public:
  InputTextWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label, size_t max_length);

  static void Initialize(IsolateData* isolate_data, v8::Local<v8::ObjectTemplate> target);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void GetText(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void SetText(const v8::FunctionCallbackInfo<v8::Value>& args);

  void Render() override;

  const std::string& text() const { return buffer_; }
  void set_text(const std::string& t);

 private:
  std::string buffer_;
  size_t max_length_;
};

class InputFloatWidget : public Widget {
 public:
  InputFloatWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label, float value);

  static void Initialize(IsolateData* isolate_data, v8::Local<v8::ObjectTemplate> target);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void GetValue(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void SetValue(const v8::FunctionCallbackInfo<v8::Value>& args);

  void Render() override;
  float value() const { return value_; }
  void set_value(float v) { value_ = v; }

 private:
  float value_;
};

class InputIntWidget : public Widget {
 public:
  InputIntWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label, int value);

  static void Initialize(IsolateData* isolate_data, v8::Local<v8::ObjectTemplate> target);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void GetValue(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void SetValue(const v8::FunctionCallbackInfo<v8::Value>& args);

  void Render() override;
  int value() const { return value_; }
  void set_value(int v) { value_ = v; }

 private:
  int value_;
};

class InputTextMultilineWidget : public Widget {
 public:
  InputTextMultilineWidget(Realm* realm,
                           v8::Local<v8::Object> object,
                           const std::string& label,
                           size_t max_length,
                           float width,
                           float height);

  static void Initialize(IsolateData* isolate_data, v8::Local<v8::ObjectTemplate> target);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void GetText(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void SetText(const v8::FunctionCallbackInfo<v8::Value>& args);

  void Render() override;
  const std::string& text() const { return buffer_; }
  void set_text(const std::string& t);

 private:
  std::string buffer_;
  size_t max_length_;
  float width_, height_;
};

class InputTextWithHintWidget : public Widget {
 public:
  InputTextWithHintWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label, size_t max_length);

  static void Initialize(IsolateData* isolate_data, v8::Local<v8::ObjectTemplate> target);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void GetText(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void SetText(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void GetHint(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void SetHint(const v8::FunctionCallbackInfo<v8::Value>& args);

  void Render() override;
  const std::string& text() const { return buffer_; }
  void set_text(const std::string& t);
  const std::string& hint() const { return buffer_; }
  void set_hint(const std::string& t);

 private:
  std::string buffer_;
  std::string hint_;
  size_t max_length_;
};

}  // namespace nyx
