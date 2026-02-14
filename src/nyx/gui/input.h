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

  void Render() override;

  const std::string& label() const { return label_; }
  void set_label(const std::string& l) { label_ = l; }
  const std::string& text() const { return buffer_; }
  void set_text(const std::string& t);

 private:
  std::string label_;
  std::string buffer_;
  size_t max_length_;
};

class InputFloatWidget : public Widget {
 public:
  InputFloatWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label, float value);
  void Render() override;
  const std::string& label() const { return label_; }
  void set_label(const std::string& l) { label_ = l; }
  float value() const { return value_; }
  void set_value(float v) { value_ = v; }

 private:
  std::string label_;
  float value_;
};

class InputIntWidget : public Widget {
 public:
  InputIntWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label, int value);
  void Render() override;
  const std::string& label() const { return label_; }
  void set_label(const std::string& l) { label_ = l; }
  int value() const { return value_; }
  void set_value(int v) { value_ = v; }

 private:
  std::string label_;
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
  void Render() override;
  const std::string& label() const { return label_; }
  void set_label(const std::string& l) { label_ = l; }
  const std::string& text() const { return buffer_; }
  void set_text(const std::string& t);

 private:
  std::string label_;
  std::string buffer_;
  size_t max_length_;
  float width_, height_;
};

}  // namespace nyx
