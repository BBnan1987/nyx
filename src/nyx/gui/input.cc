#include "nyx/gui/input.h"

namespace nyx {

using v8::HandleScope;
using v8::Isolate;

InputTextWidget::InputTextWidget(Realm* realm,
                                 v8::Local<v8::Object> object,
                                 const std::string& label,
                                 size_t max_length)
    : Widget(realm, object), label_(label), max_length_(max_length) {
  buffer_.reserve(max_length_);
}

void InputTextWidget::set_text(const std::string& t) {
  buffer_ = t;
  if (buffer_.size() > max_length_) {
    buffer_.resize(max_length_);
  }
}

void InputTextWidget::Render() {
  // ImGui::InputText needs a mutable char buffer with fixed capacity
  std::vector<char> buf(max_length_ + 1, '\0');
  std::copy(buffer_.begin(), buffer_.end(), buf.begin());

  if (ImGui::InputText(label_.c_str(), buf.data(), max_length_ + 1)) {
    std::string new_text(buf.data());
    if (new_text != buffer_) {
      buffer_ = std::move(new_text);
      Isolate* iso = isolate();
      HandleScope scope(iso);
      EmitEvent("change", v8::String::NewFromUtf8(iso, buffer_.c_str()).ToLocalChecked());
    }
  }
}

InputFloatWidget::InputFloatWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label, float value)
    : Widget(realm, object), label_(label), value_(value) {}

void InputFloatWidget::Render() {
  float old = value_;
  ImGui::InputFloat(label_.c_str(), &value_);
  if (value_ != old) {
    Isolate* iso = isolate();
    HandleScope scope(iso);
    EmitEvent("change", v8::Number::New(iso, value_));
  }
}

InputIntWidget::InputIntWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label, int value)
    : Widget(realm, object), label_(label), value_(value) {}

void InputIntWidget::Render() {
  int old = value_;
  ImGui::InputInt(label_.c_str(), &value_);
  if (value_ != old) {
    Isolate* iso = isolate();
    HandleScope scope(iso);
    EmitEvent("change", v8::Integer::New(iso, value_));
  }
}

InputTextMultilineWidget::InputTextMultilineWidget(
    Realm* realm, v8::Local<v8::Object> object, const std::string& label, size_t max_length, float width, float height)
    : Widget(realm, object), label_(label), max_length_(max_length), width_(width), height_(height) {
  buffer_.reserve(max_length_);
}

void InputTextMultilineWidget::set_text(const std::string& t) {
  buffer_ = t;
  if (buffer_.size() > max_length_) {
    buffer_.resize(max_length_);
  }
}

void InputTextMultilineWidget::Render() {
  std::vector<char> buf(max_length_ + 1, '\0');
  std::copy(buffer_.begin(), buffer_.end(), buf.begin());

  if (ImGui::InputTextMultiline(label_.c_str(), buf.data(), max_length_ + 1, ImVec2(width_, height_))) {
    std::string new_text(buf.data());
    if (new_text != buffer_) {
      buffer_ = std::move(new_text);
      Isolate* iso = isolate();
      HandleScope scope(iso);
      EmitEvent("change", v8::String::NewFromUtf8(iso, buffer_.c_str()).ToLocalChecked());
    }
  }
}

}  // namespace nyx
