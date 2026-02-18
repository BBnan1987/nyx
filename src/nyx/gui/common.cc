#include "nyx/gui/common.h"

#include "nyx/env.h"

namespace nyx {

using v8::Boolean;
using v8::Context;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::HandleScope;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::ObjectTemplate;
using v8::String;
using v8::Value;

ButtonWidget::ButtonWidget(Realm* realm, Local<Object> object, const std::string& label, float width, float height)
    : Widget(realm, object, label), width_(width), height_(height) {}

void ButtonWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  SetProtoProperty(isolate, tmpl, "clicked", GetClicked);

  tmpl->SetClassName(FixedOneByteString(isolate, "Button"));
  target->Set(FixedOneByteString(isolate, "Button"), tmpl);
}

void ButtonWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value label(isolate, args[0]);
  float w = args.Length() > 1 ? static_cast<float>(args[1]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  float h = args.Length() > 2 ? static_cast<float>(args[2]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  new ButtonWidget(env->principal_realm(), args.This(), *label, w, h);
}

void ButtonWidget::GetClicked(const FunctionCallbackInfo<Value>& args) {
  ButtonWidget* self = BaseObject::Unwrap<ButtonWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->clicked());
}

void ButtonWidget::Render() {
  clicked_ = ImGui::Button(label_.c_str(), ImVec2(width_, height_));
  if (clicked_) {
    EmitEvent("click");
  }
}

InvisibleButtonWidget::InvisibleButtonWidget(
    Realm* realm, Local<Object> object, const std::string& label, float width, float height)
    : Widget(realm, object, label), width_(width), height_(height) {}

void InvisibleButtonWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  SetProtoProperty(isolate, tmpl, "clicked", GetClicked);

  tmpl->SetClassName(FixedOneByteString(isolate, "InvisibleButton"));
  target->Set(FixedOneByteString(isolate, "InvisibleButton"), tmpl);
}

void InvisibleButtonWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value label(isolate, args[0]);
  float w = args.Length() > 1 ? static_cast<float>(args[1]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  float h = args.Length() > 2 ? static_cast<float>(args[2]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  new InvisibleButtonWidget(env->principal_realm(), args.This(), *label, w, h);
}

void InvisibleButtonWidget::GetClicked(const FunctionCallbackInfo<Value>& args) {
  InvisibleButtonWidget* self = BaseObject::Unwrap<InvisibleButtonWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->clicked());
}

void InvisibleButtonWidget::Render() {
  clicked_ = ImGui::InvisibleButton(label_.c_str(), ImVec2(width_, height_));
  if (clicked_) {
    EmitEvent("click");
  }
}

CheckboxWidget::CheckboxWidget(Realm* realm, Local<Object> object, const std::string& label, bool checked)
    : Widget(realm, object, label), checked_(checked) {}

void CheckboxWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  SetProtoProperty(isolate, tmpl, "checked", GetChecked, SetChecked);

  tmpl->SetClassName(FixedOneByteString(isolate, "Checkbox"));
  target->Set(FixedOneByteString(isolate, "Checkbox"), tmpl);
}

void CheckboxWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Utf8Value label(isolate, args[0]);
  bool checked = args.Length() > 1 ? args[1]->BooleanValue(isolate) : false;
  new CheckboxWidget(env->principal_realm(), args.This(), *label, checked);
}

void CheckboxWidget::GetChecked(const FunctionCallbackInfo<Value>& args) {
  CheckboxWidget* self = BaseObject::Unwrap<CheckboxWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->checked());
}

void CheckboxWidget::SetChecked(const FunctionCallbackInfo<Value>& args) {
  CheckboxWidget* self = BaseObject::Unwrap<CheckboxWidget>(args.This());
  if (self) self->set_checked(args[0]->BooleanValue(args.GetIsolate()));
}

void CheckboxWidget::Render() {
  bool old = checked_;
  ImGui::Checkbox(label_.c_str(), &checked_);
  if (checked_ != old) {
    Isolate* iso = isolate();
    HandleScope scope(iso);
    EmitEvent("change", Boolean::New(iso, checked_));
  }
}

void BulletWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  tmpl->SetClassName(FixedOneByteString(isolate, "Bullet"));
  target->Set(FixedOneByteString(isolate, "Bullet"), tmpl);
}

void BulletWidget::New(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args.GetIsolate());
  new BulletWidget(env->principal_realm(), args.This());
}

void BulletWidget::Render() {
  ImGui::Bullet();
}

SmallButtonWidget::SmallButtonWidget(Realm* realm, Local<Object> object, const std::string& label)
    : Widget(realm, object, label) {}

void SmallButtonWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  SetProtoProperty(isolate, tmpl, "clicked", GetClicked);

  tmpl->SetClassName(FixedOneByteString(isolate, "SmallButton"));
  target->Set(FixedOneByteString(isolate, "SmallButton"), tmpl);
}

void SmallButtonWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Utf8Value label(isolate, args[0]);
  new SmallButtonWidget(env->principal_realm(), args.This(), *label);
}

void SmallButtonWidget::GetClicked(const FunctionCallbackInfo<Value>& args) {
  SmallButtonWidget* self = BaseObject::Unwrap<SmallButtonWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->clicked());
}

void SmallButtonWidget::Render() {
  clicked_ = ImGui::SmallButton(label_.c_str());
  if (clicked_) {
    EmitEvent("click");
  }
}

ArrowButtonWidget::ArrowButtonWidget(Realm* realm, Local<Object> object, const std::string& id, ImGuiDir dir)
    : Widget(realm, object), id_(id), dir_(dir) {}

void ArrowButtonWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  SetProtoProperty(isolate, tmpl, "clicked", GetClicked);

  tmpl->SetClassName(FixedOneByteString(isolate, "ArrowButton"));
  target->Set(FixedOneByteString(isolate, "ArrowButton"), tmpl);
}

void ArrowButtonWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value id(isolate, args[0]);
  ImGuiDir dir = static_cast<ImGuiDir>(args[1]->Int32Value(context).FromMaybe(0));
  new ArrowButtonWidget(env->principal_realm(), args.This(), *id, dir);
}

void ArrowButtonWidget::GetClicked(const FunctionCallbackInfo<Value>& args) {
  ArrowButtonWidget* self = BaseObject::Unwrap<ArrowButtonWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->clicked());
}

void ArrowButtonWidget::Render() {
  clicked_ = ImGui::ArrowButton(id_.c_str(), dir_);
  if (clicked_) {
    EmitEvent("click");
  }
}

RadioButtonWidget::RadioButtonWidget(Realm* realm, Local<Object> object, const std::string& label, bool active)
    : Widget(realm, object, label), active_(active) {}

void RadioButtonWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  SetProtoProperty(isolate, tmpl, "active", GetActive, SetActive);
  SetProtoProperty(isolate, tmpl, "clicked", GetClicked);

  tmpl->SetClassName(FixedOneByteString(isolate, "RadioButton"));
  target->Set(FixedOneByteString(isolate, "RadioButton"), tmpl);
}

void RadioButtonWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Utf8Value label(isolate, args[0]);
  bool active = args.Length() > 1 ? args[1]->BooleanValue(isolate) : false;
  new RadioButtonWidget(env->principal_realm(), args.This(), *label, active);
}

void RadioButtonWidget::GetActive(const FunctionCallbackInfo<Value>& args) {
  RadioButtonWidget* self = BaseObject::Unwrap<RadioButtonWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->active());
}

void RadioButtonWidget::SetActive(const FunctionCallbackInfo<Value>& args) {
  RadioButtonWidget* self = BaseObject::Unwrap<RadioButtonWidget>(args.This());
  if (self) self->set_active(args[0]->BooleanValue(args.GetIsolate()));
}

void RadioButtonWidget::GetClicked(const FunctionCallbackInfo<Value>& args) {
  RadioButtonWidget* self = BaseObject::Unwrap<RadioButtonWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->clicked());
}

void RadioButtonWidget::Render() {
  clicked_ = ImGui::RadioButton(label_.c_str(), active_);
  if (clicked_) {
    EmitEvent("click");
  }
}

ProgressBarWidget::ProgressBarWidget(Realm* realm, Local<Object> object, float fraction, const std::string& overlay)
    : Widget(realm, object), fraction_(fraction), overlay_(overlay) {}

void ProgressBarWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  SetProtoProperty(isolate, tmpl, "fraction", GetFraction, SetFraction);
  SetProtoProperty(isolate, tmpl, "overlay", GetOverlay, SetOverlay);

  tmpl->SetClassName(FixedOneByteString(isolate, "ProgressBar"));
  target->Set(FixedOneByteString(isolate, "ProgressBar"), tmpl);
}

void ProgressBarWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  float fraction = args.Length() > 0 ? static_cast<float>(args[0]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  std::string overlay;
  if (args.Length() > 1 && !args[1]->IsUndefined()) {
    Utf8Value ov(isolate, args[1]);
    overlay = *ov;
  }
  new ProgressBarWidget(env->principal_realm(), args.This(), fraction, overlay);
}

void ProgressBarWidget::GetFraction(const FunctionCallbackInfo<Value>& args) {
  ProgressBarWidget* self = BaseObject::Unwrap<ProgressBarWidget>(args.This());
  if (self) args.GetReturnValue().Set(static_cast<double>(self->fraction()));
}

void ProgressBarWidget::SetFraction(const FunctionCallbackInfo<Value>& args) {
  ProgressBarWidget* self = BaseObject::Unwrap<ProgressBarWidget>(args.This());
  if (self) {
    Local<Context> ctx = args.GetIsolate()->GetCurrentContext();
    self->set_fraction(static_cast<float>(args[0]->NumberValue(ctx).FromMaybe(0.0)));
  }
}

void ProgressBarWidget::GetOverlay(const FunctionCallbackInfo<Value>& args) {
  ProgressBarWidget* self = BaseObject::Unwrap<ProgressBarWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->overlay().c_str()).ToLocalChecked());
  }
}

void ProgressBarWidget::SetOverlay(const FunctionCallbackInfo<Value>& args) {
  ProgressBarWidget* self = BaseObject::Unwrap<ProgressBarWidget>(args.This());
  if (self) {
    Utf8Value ov(args.GetIsolate(), args[0]);
    self->set_overlay(*ov);
  }
}

void ProgressBarWidget::Render() {
  ImGui::ProgressBar(fraction_, ImVec2(-FLT_MIN, 0), overlay_.empty() ? nullptr : overlay_.c_str());
}

}  // namespace nyx
