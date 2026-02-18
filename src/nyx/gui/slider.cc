#include "nyx/gui/slider.h"

#include "nyx/env.h"

namespace nyx {

using v8::Context;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::HandleScope;
using v8::Integer;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::ObjectTemplate;
using v8::String;
using v8::Value;

SliderFloatWidget::SliderFloatWidget(
    Realm* realm, Local<Object> object, const std::string& label, float min, float max, float value)
    : Widget(realm, object, label), value_(value), min_(min), max_(max) {}

void SliderFloatWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  SetProtoProperty(isolate, tmpl, "value", GetValue, SetValue);

  tmpl->SetClassName(FixedOneByteString(isolate, "SliderFloat"));
  target->Set(FixedOneByteString(isolate, "SliderFloat"), tmpl);
}

void SliderFloatWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value label(isolate, args[0]);
  float min_val = static_cast<float>(args[1]->NumberValue(context).FromMaybe(0.0));
  float max_val = static_cast<float>(args[2]->NumberValue(context).FromMaybe(1.0));
  float value = args.Length() > 3 ? static_cast<float>(args[3]->NumberValue(context).FromMaybe(0.0)) : min_val;
  new SliderFloatWidget(env->principal_realm(), args.This(), *label, min_val, max_val, value);
}

void SliderFloatWidget::GetValue(const FunctionCallbackInfo<Value>& args) {
  SliderFloatWidget* self = BaseObject::Unwrap<SliderFloatWidget>(args.This());
  if (self) args.GetReturnValue().Set(static_cast<double>(self->value()));
}

void SliderFloatWidget::SetValue(const FunctionCallbackInfo<Value>& args) {
  SliderFloatWidget* self = BaseObject::Unwrap<SliderFloatWidget>(args.This());
  if (self) {
    Local<Context> ctx = args.GetIsolate()->GetCurrentContext();
    self->set_value(static_cast<float>(args[0]->NumberValue(ctx).FromMaybe(0.0)));
  }
}

void SliderFloatWidget::Render() {
  float old = value_;
  ImGui::SliderFloat(label_.c_str(), &value_, min_, max_);
  if (value_ != old) {
    Isolate* iso = isolate();
    HandleScope scope(iso);
    EmitEvent("change", Number::New(iso, value_));
  }
}

SliderIntWidget::SliderIntWidget(
    Realm* realm, Local<Object> object, const std::string& label, int min, int max, int value)
    : Widget(realm, object, label), value_(value), min_(min), max_(max) {}

void SliderIntWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  SetProtoProperty(isolate, tmpl, "value", GetValue, SetValue);

  tmpl->SetClassName(FixedOneByteString(isolate, "SliderInt"));
  target->Set(FixedOneByteString(isolate, "SliderInt"), tmpl);
}

void SliderIntWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value label(isolate, args[0]);
  int min_val = args[1]->Int32Value(context).FromMaybe(0);
  int max_val = args[2]->Int32Value(context).FromMaybe(100);
  int value = args.Length() > 3 ? args[3]->Int32Value(context).FromMaybe(min_val) : min_val;
  new SliderIntWidget(env->principal_realm(), args.This(), *label, min_val, max_val, value);
}

void SliderIntWidget::GetValue(const FunctionCallbackInfo<Value>& args) {
  SliderIntWidget* self = BaseObject::Unwrap<SliderIntWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->value());
}

void SliderIntWidget::SetValue(const FunctionCallbackInfo<Value>& args) {
  SliderIntWidget* self = BaseObject::Unwrap<SliderIntWidget>(args.This());
  if (self) {
    Local<Context> ctx = args.GetIsolate()->GetCurrentContext();
    self->set_value(args[0]->Int32Value(ctx).FromMaybe(0));
  }
}

void SliderIntWidget::Render() {
  int old = value_;
  ImGui::SliderInt(label_.c_str(), &value_, min_, max_);
  if (value_ != old) {
    Isolate* iso = isolate();
    HandleScope scope(iso);
    EmitEvent("change", Integer::New(iso, value_));
  }
}

DragFloatWidget::DragFloatWidget(
    Realm* realm, Local<Object> object, const std::string& label, float value, float speed, float min, float max)
    : Widget(realm, object, label), value_(value), speed_(speed), min_(min), max_(max) {}

void DragFloatWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  SetProtoProperty(isolate, tmpl, "value", GetValue, SetValue);

  tmpl->SetClassName(FixedOneByteString(isolate, "DragFloat"));
  target->Set(FixedOneByteString(isolate, "DragFloat"), tmpl);
}

void DragFloatWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value label(isolate, args[0]);
  float value = static_cast<float>(args[1]->NumberValue(context).FromMaybe(0.0));
  float speed = args.Length() > 2 ? static_cast<float>(args[2]->NumberValue(context).FromMaybe(1.0)) : 1.0f;
  float min_val = args.Length() > 3 ? static_cast<float>(args[3]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  float max_val = args.Length() > 4 ? static_cast<float>(args[4]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  new DragFloatWidget(env->principal_realm(), args.This(), *label, value, speed, min_val, max_val);
}

void DragFloatWidget::GetValue(const FunctionCallbackInfo<Value>& args) {
  DragFloatWidget* self = BaseObject::Unwrap<DragFloatWidget>(args.This());
  if (self) args.GetReturnValue().Set(static_cast<double>(self->value()));
}

void DragFloatWidget::SetValue(const FunctionCallbackInfo<Value>& args) {
  DragFloatWidget* self = BaseObject::Unwrap<DragFloatWidget>(args.This());
  if (self) {
    Local<Context> ctx = args.GetIsolate()->GetCurrentContext();
    self->set_value(static_cast<float>(args[0]->NumberValue(ctx).FromMaybe(0.0)));
  }
}

void DragFloatWidget::Render() {
  float old = value_;
  ImGui::DragFloat(label_.c_str(), &value_, speed_, min_, max_);
  if (value_ != old) {
    Isolate* iso = isolate();
    HandleScope scope(iso);
    EmitEvent("change", Number::New(iso, value_));
  }
}

DragIntWidget::DragIntWidget(
    Realm* realm, Local<Object> object, const std::string& label, int value, float speed, int min, int max)
    : Widget(realm, object, label), value_(value), speed_(speed), min_(min), max_(max) {}

void DragIntWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  SetProtoProperty(isolate, tmpl, "value", GetValue, SetValue);

  tmpl->SetClassName(FixedOneByteString(isolate, "DragInt"));
  target->Set(FixedOneByteString(isolate, "DragInt"), tmpl);
}

void DragIntWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value label(isolate, args[0]);
  int value = args[1]->Int32Value(context).FromMaybe(0);
  float speed = args.Length() > 2 ? static_cast<float>(args[2]->NumberValue(context).FromMaybe(1.0)) : 1.0f;
  int min_val = args.Length() > 3 ? args[3]->Int32Value(context).FromMaybe(0) : 0;
  int max_val = args.Length() > 4 ? args[4]->Int32Value(context).FromMaybe(0) : 0;
  new DragIntWidget(env->principal_realm(), args.This(), *label, value, speed, min_val, max_val);
}

void DragIntWidget::GetValue(const FunctionCallbackInfo<Value>& args) {
  DragIntWidget* self = BaseObject::Unwrap<DragIntWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->value());
}

void DragIntWidget::SetValue(const FunctionCallbackInfo<Value>& args) {
  DragIntWidget* self = BaseObject::Unwrap<DragIntWidget>(args.This());
  if (self) {
    Local<Context> ctx = args.GetIsolate()->GetCurrentContext();
    self->set_value(args[0]->Int32Value(ctx).FromMaybe(0));
  }
}

void DragIntWidget::Render() {
  int old = value_;
  ImGui::DragInt(label_.c_str(), &value_, speed_, min_, max_);
  if (value_ != old) {
    Isolate* iso = isolate();
    HandleScope scope(iso);
    EmitEvent("change", Integer::New(iso, value_));
  }
}

}  // namespace nyx
