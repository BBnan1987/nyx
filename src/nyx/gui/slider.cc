#include "nyx/gui/slider.h"

namespace nyx {

using v8::HandleScope;
using v8::Isolate;

SliderFloatWidget::SliderFloatWidget(
    Realm* realm, v8::Local<v8::Object> object, const std::string& label, float min, float max, float value)
    : Widget(realm, object), label_(label), value_(value), min_(min), max_(max) {}

void SliderFloatWidget::Render() {
  float old = value_;
  ImGui::SliderFloat(label_.c_str(), &value_, min_, max_);
  if (value_ != old) {
    Isolate* iso = isolate();
    HandleScope scope(iso);
    EmitEvent("change", v8::Number::New(iso, value_));
  }
}

SliderIntWidget::SliderIntWidget(
    Realm* realm, v8::Local<v8::Object> object, const std::string& label, int min, int max, int value)
    : Widget(realm, object), label_(label), value_(value), min_(min), max_(max) {}

void SliderIntWidget::Render() {
  int old = value_;
  ImGui::SliderInt(label_.c_str(), &value_, min_, max_);
  if (value_ != old) {
    Isolate* iso = isolate();
    HandleScope scope(iso);
    EmitEvent("change", v8::Integer::New(iso, value_));
  }
}

DragFloatWidget::DragFloatWidget(Realm* realm,
                                 v8::Local<v8::Object> object,
                                 const std::string& label,
                                 float value,
                                 float speed,
                                 float min,
                                 float max)
    : Widget(realm, object), label_(label), value_(value), speed_(speed), min_(min), max_(max) {}

void DragFloatWidget::Render() {
  float old = value_;
  ImGui::DragFloat(label_.c_str(), &value_, speed_, min_, max_);
  if (value_ != old) {
    Isolate* iso = isolate();
    HandleScope scope(iso);
    EmitEvent("change", v8::Number::New(iso, value_));
  }
}

DragIntWidget::DragIntWidget(
    Realm* realm, v8::Local<v8::Object> object, const std::string& label, int value, float speed, int min, int max)
    : Widget(realm, object), label_(label), value_(value), speed_(speed), min_(min), max_(max) {}

void DragIntWidget::Render() {
  int old = value_;
  ImGui::DragInt(label_.c_str(), &value_, speed_, min_, max_);
  if (value_ != old) {
    Isolate* iso = isolate();
    HandleScope scope(iso);
    EmitEvent("change", v8::Integer::New(iso, value_));
  }
}

}  // namespace nyx
