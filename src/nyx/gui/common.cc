#include "nyx/gui/common.h"

namespace nyx {

using v8::HandleScope;
using v8::Isolate;

ButtonWidget::ButtonWidget(
    Realm* realm, v8::Local<v8::Object> object, const std::string& label, float width, float height)
    : Widget(realm, object), label_(label), width_(width), height_(height) {}

void ButtonWidget::Render() {
  clicked_ = ImGui::Button(label_.c_str(), ImVec2(width_, height_));
  if (clicked_) {
    EmitEvent("click");
  }
}

InvisibleButtonWidget::InvisibleButtonWidget(
    Realm* realm, v8::Local<v8::Object> object, const std::string& label, float width, float height)
    : Widget(realm, object), label_(label), width_(width), height_(height) {}

void InvisibleButtonWidget::Render() {
  clicked_ = ImGui::InvisibleButton(label_.c_str(), ImVec2(width_, height_));
  if (clicked_) {
    EmitEvent("click");
  }
}

CheckboxWidget::CheckboxWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label, bool checked)
    : Widget(realm, object), label_(label), checked_(checked) {}

void CheckboxWidget::Render() {
  bool old = checked_;
  ImGui::Checkbox(label_.c_str(), &checked_);
  if (checked_ != old) {
    Isolate* iso = isolate();
    HandleScope scope(iso);
    EmitEvent("change", v8::Boolean::New(iso, checked_));
  }
}

void BulletWidget::Render() {
  ImGui::Bullet();
}

SmallButtonWidget::SmallButtonWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label)
    : Widget(realm, object), label_(label) {}

void SmallButtonWidget::Render() {
  clicked_ = ImGui::SmallButton(label_.c_str());
  if (clicked_) {
    EmitEvent("click");
  }
}

ArrowButtonWidget::ArrowButtonWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& id, ImGuiDir dir)
    : Widget(realm, object), id_(id), dir_(dir) {}

void ArrowButtonWidget::Render() {
  clicked_ = ImGui::ArrowButton(id_.c_str(), dir_);
  if (clicked_) {
    EmitEvent("click");
  }
}

RadioButtonWidget::RadioButtonWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label, bool active)
    : Widget(realm, object), label_(label), active_(active) {}

void RadioButtonWidget::Render() {
  clicked_ = ImGui::RadioButton(label_.c_str(), active_);
  if (clicked_) {
    EmitEvent("click");
  }
}

ProgressBarWidget::ProgressBarWidget(Realm* realm,
                                     v8::Local<v8::Object> object,
                                     float fraction,
                                     const std::string& overlay)
    : Widget(realm, object), fraction_(fraction), overlay_(overlay) {}

void ProgressBarWidget::Render() {
  ImGui::ProgressBar(fraction_, ImVec2(-FLT_MIN, 0), overlay_.empty() ? nullptr : overlay_.c_str());
}

}  // namespace nyx
