#include "nyx/gui/input.h"

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

InputTextWidget::InputTextWidget(Realm* realm, Local<Object> object, const std::string& label, size_t max_length)
    : Widget(realm, object, label), max_length_(max_length) {
  buffer_.reserve(max_length_);
}

void InputTextWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  SetProtoProperty(isolate, tmpl, "text", GetText, SetText);

  tmpl->SetClassName(FixedOneByteString(isolate, "InputText"));
  target->Set(FixedOneByteString(isolate, "InputText"), tmpl);
}

void InputTextWidget::set_text(const std::string& t) {
  buffer_ = t;
  if (buffer_.size() > max_length_) {
    buffer_.resize(max_length_);
  }
}

void InputTextWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value label(isolate, args[0]);
  size_t max_length = args.Length() > 1 ? static_cast<size_t>(args[1]->Uint32Value(context).FromMaybe(256)) : 256;
  new InputTextWidget(env->principal_realm(), args.This(), *label, max_length);
}

void InputTextWidget::GetText(const FunctionCallbackInfo<Value>& args) {
  InputTextWidget* self = BaseObject::Unwrap<InputTextWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->text().c_str()).ToLocalChecked());
  }
}

void InputTextWidget::SetText(const FunctionCallbackInfo<Value>& args) {
  InputTextWidget* self = BaseObject::Unwrap<InputTextWidget>(args.This());
  if (self) {
    Utf8Value text(args.GetIsolate(), args[0]);
    self->set_text(*text);
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
      EmitEvent("change", String::NewFromUtf8(iso, buffer_.c_str()).ToLocalChecked());
    }
  }
}

InputFloatWidget::InputFloatWidget(Realm* realm, Local<Object> object, const std::string& label, float value)
    : Widget(realm, object, label), value_(value) {}

void InputFloatWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  SetProtoProperty(isolate, tmpl, "value", GetValue, SetValue);

  tmpl->SetClassName(FixedOneByteString(isolate, "InputFloat"));
  target->Set(FixedOneByteString(isolate, "InputFloat"), tmpl);
}

void InputFloatWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value label(isolate, args[0]);
  float value = args.Length() > 1 ? static_cast<float>(args[1]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  new InputFloatWidget(env->principal_realm(), args.This(), *label, value);
}

void InputFloatWidget::GetValue(const FunctionCallbackInfo<Value>& args) {
  InputFloatWidget* self = BaseObject::Unwrap<InputFloatWidget>(args.This());
  if (self) args.GetReturnValue().Set(static_cast<double>(self->value()));
}

void InputFloatWidget::SetValue(const FunctionCallbackInfo<Value>& args) {
  InputFloatWidget* self = BaseObject::Unwrap<InputFloatWidget>(args.This());
  if (self) {
    Local<Context> ctx = args.GetIsolate()->GetCurrentContext();
    self->set_value(static_cast<float>(args[0]->NumberValue(ctx).FromMaybe(0.0)));
  }
}

void InputFloatWidget::Render() {
  float old = value_;
  ImGui::InputFloat(label_.c_str(), &value_);
  if (value_ != old) {
    Isolate* iso = isolate();
    HandleScope scope(iso);
    EmitEvent("change", Number::New(iso, value_));
  }
}

InputIntWidget::InputIntWidget(Realm* realm, Local<Object> object, const std::string& label, int value)
    : Widget(realm, object, label), value_(value) {}

void InputIntWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  SetProtoProperty(isolate, tmpl, "value", GetValue, SetValue);

  tmpl->SetClassName(FixedOneByteString(isolate, "InputInt"));
  target->Set(FixedOneByteString(isolate, "InputInt"), tmpl);
}

void InputIntWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value label(isolate, args[0]);
  int value = args.Length() > 1 ? args[1]->Int32Value(context).FromMaybe(0) : 0;
  new InputIntWidget(env->principal_realm(), args.This(), *label, value);
}

void InputIntWidget::GetValue(const FunctionCallbackInfo<Value>& args) {
  InputIntWidget* self = BaseObject::Unwrap<InputIntWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->value());
}

void InputIntWidget::SetValue(const FunctionCallbackInfo<Value>& args) {
  InputIntWidget* self = BaseObject::Unwrap<InputIntWidget>(args.This());
  if (self) {
    Local<Context> ctx = args.GetIsolate()->GetCurrentContext();
    self->set_value(args[0]->Int32Value(ctx).FromMaybe(0));
  }
}

void InputIntWidget::Render() {
  int old = value_;
  ImGui::InputInt(label_.c_str(), &value_);
  if (value_ != old) {
    Isolate* iso = isolate();
    HandleScope scope(iso);
    EmitEvent("change", Integer::New(iso, value_));
  }
}

InputTextMultilineWidget::InputTextMultilineWidget(
    Realm* realm, Local<Object> object, const std::string& label, size_t max_length, float width, float height)
    : Widget(realm, object, label), max_length_(max_length), width_(width), height_(height) {
  buffer_.reserve(max_length_);
}

void InputTextMultilineWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  SetProtoProperty(isolate, tmpl, "text", GetText, SetText);

  tmpl->SetClassName(FixedOneByteString(isolate, "InputTextMultiline"));
  target->Set(FixedOneByteString(isolate, "InputTextMultiline"), tmpl);
}

void InputTextMultilineWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value label(isolate, args[0]);
  size_t max_length = args.Length() > 1 ? static_cast<size_t>(args[1]->Uint32Value(context).FromMaybe(1024)) : 1024;
  float width = args.Length() > 2 ? static_cast<float>(args[2]->NumberValue(context).FromMaybe(-1.0)) : -1.0f;
  float height = args.Length() > 3 ? static_cast<float>(args[3]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  new InputTextMultilineWidget(env->principal_realm(), args.This(), *label, max_length, width, height);
}

void InputTextMultilineWidget::GetText(const FunctionCallbackInfo<Value>& args) {
  InputTextMultilineWidget* self = BaseObject::Unwrap<InputTextMultilineWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->text().c_str()).ToLocalChecked());
  }
}

void InputTextMultilineWidget::SetText(const FunctionCallbackInfo<Value>& args) {
  InputTextMultilineWidget* self = BaseObject::Unwrap<InputTextMultilineWidget>(args.This());
  if (self) {
    Utf8Value text(args.GetIsolate(), args[0]);
    self->set_text(*text);
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
      EmitEvent("change", String::NewFromUtf8(iso, buffer_.c_str()).ToLocalChecked());
    }
  }
}

void InputTextMultilineWidget::set_text(const std::string& t) {
  buffer_ = t;
  if (buffer_.size() > max_length_) {
    buffer_.resize(max_length_);
  }
}

InputTextWithHintWidget::InputTextWithHintWidget(Realm* realm,
                                                 Local<Object> object,
                                                 const std::string& label,
                                                 size_t max_length)
    : Widget(realm, object, label), max_length_(max_length) {
  buffer_.reserve(max_length_);
}

void InputTextWithHintWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  SetProtoProperty(isolate, tmpl, "text", GetText, SetText);
  SetProtoProperty(isolate, tmpl, "hint", GetHint, SetHint);

  tmpl->SetClassName(FixedOneByteString(isolate, "InputTextWithHint"));
  target->Set(FixedOneByteString(isolate, "InputTextWithHint"), tmpl);
}

void InputTextWithHintWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value label(isolate, args[0]);
  size_t max_length = args.Length() > 1 ? static_cast<size_t>(args[1]->Uint32Value(context).FromMaybe(256)) : 256;
  new InputTextWithHintWidget(env->principal_realm(), args.This(), *label, max_length);
}

void InputTextWithHintWidget::GetText(const FunctionCallbackInfo<Value>& args) {
  InputTextWithHintWidget* self = BaseObject::Unwrap<InputTextWithHintWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->text().c_str()).ToLocalChecked());
  }
}

void InputTextWithHintWidget::SetText(const FunctionCallbackInfo<Value>& args) {
  InputTextWithHintWidget* self = BaseObject::Unwrap<InputTextWithHintWidget>(args.This());
  if (self) {
    Utf8Value text(args.GetIsolate(), args[0]);
    self->set_text(*text);
  }
}

void InputTextWithHintWidget::GetHint(const FunctionCallbackInfo<Value>& args) {
  InputTextWithHintWidget* self = BaseObject::Unwrap<InputTextWithHintWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->hint().c_str()).ToLocalChecked());
  }
}

void InputTextWithHintWidget::SetHint(const FunctionCallbackInfo<Value>& args) {
  InputTextWithHintWidget* self = BaseObject::Unwrap<InputTextWithHintWidget>(args.This());
  if (self) {
    Utf8Value hint(args.GetIsolate(), args[0]);
    self->set_hint(*hint);
  }
}

void InputTextWithHintWidget::Render() {
  std::vector<char> buf(max_length_ + 1, '\0');
  std::copy(buffer_.begin(), buffer_.end(), buf.begin());

  if (ImGui::InputTextWithHint(label_.c_str(), hint_.c_str(), buf.data(), max_length_ + 1)) {
    std::string new_text(buf.data());
    if (new_text != buffer_) {
      buffer_ = std::move(new_text);
      Isolate* iso = isolate();
      HandleScope scope(iso);
      EmitEvent("change", String::NewFromUtf8(iso, buffer_.c_str()).ToLocalChecked());
    }
  }
}

void InputTextWithHintWidget::set_text(const std::string& t) {
  buffer_ = t;
  if (buffer_.size() > max_length_) {
    buffer_.resize(max_length_);
  }
}

void InputTextWithHintWidget::set_hint(const std::string& t) {
  hint_ = t;
}

}  // namespace nyx
