#include "nyx/gui/text.h"

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

TextWidget::TextWidget(Realm* realm, Local<Object> object, const std::string& text)
    : Widget(realm, object), text_(text) {}

void TextWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  SetProtoProperty(isolate, tmpl, "text", GetText, SetText);

  tmpl->SetClassName(FixedOneByteString(isolate, "Text"));
  target->Set(FixedOneByteString(isolate, "Text"), tmpl);
}

void TextWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Utf8Value text(isolate, args[0]);
  new TextWidget(env->principal_realm(), args.This(), *text);
}

void TextWidget::GetText(const FunctionCallbackInfo<Value>& args) {
  TextWidget* self = BaseObject::Unwrap<TextWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->text().c_str()).ToLocalChecked());
  }
}

void TextWidget::SetText(const FunctionCallbackInfo<Value>& args) {
  TextWidget* self = BaseObject::Unwrap<TextWidget>(args.This());
  if (self) {
    Utf8Value text(args.GetIsolate(), args[0]);
    self->set_text(*text);
  }
}

void TextWidget::Render() {
  ImGui::TextUnformatted(text_.c_str());
}

TextColoredWidget::TextColoredWidget(
    Realm* realm, Local<Object> object, const std::string& text, float r, float g, float b, float a)
    : Widget(realm, object), text_(text), r_(r), g_(g), b_(b), a_(a) {}

void TextColoredWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  SetProtoProperty(isolate, tmpl, "text", GetText, SetText);

  tmpl->SetClassName(FixedOneByteString(isolate, "TextColored"));
  target->Set(FixedOneByteString(isolate, "TextColored"), tmpl);
}

void TextColoredWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value text(isolate, args[0]);
  float r = static_cast<float>(args[1]->NumberValue(context).FromMaybe(1.0));
  float g = static_cast<float>(args[2]->NumberValue(context).FromMaybe(1.0));
  float b = static_cast<float>(args[3]->NumberValue(context).FromMaybe(1.0));
  float a = static_cast<float>(args[4]->NumberValue(context).FromMaybe(1.0));
  new TextColoredWidget(env->principal_realm(), args.This(), *text, r, g, b, a);
}

void TextColoredWidget::GetText(const FunctionCallbackInfo<Value>& args) {
  TextWidget* self = BaseObject::Unwrap<TextWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->text().c_str()).ToLocalChecked());
  }
}

void TextColoredWidget::SetText(const FunctionCallbackInfo<Value>& args) {
  TextWidget* self = BaseObject::Unwrap<TextWidget>(args.This());
  if (self) {
    Utf8Value text(args.GetIsolate(), args[0]);
    self->set_text(*text);
  }
}

void TextColoredWidget::Render() {
  ImGui::TextColored(ImVec4(r_, g_, b_, a_), "%s", text_.c_str());
}

TextWrappedWidget::TextWrappedWidget(Realm* realm, Local<Object> object, const std::string& text)
    : Widget(realm, object), text_(text) {}

void TextWrappedWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  SetProtoProperty(isolate, tmpl, "text", GetText, SetText);

  tmpl->SetClassName(FixedOneByteString(isolate, "TextWrapped"));
  target->Set(FixedOneByteString(isolate, "TextWrapped"), tmpl);
}

void TextWrappedWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Utf8Value text(isolate, args[0]);
  new TextWrappedWidget(env->principal_realm(), args.This(), *text);
}

void TextWrappedWidget::GetText(const FunctionCallbackInfo<Value>& args) {
  TextWidget* self = BaseObject::Unwrap<TextWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->text().c_str()).ToLocalChecked());
  }
}

void TextWrappedWidget::SetText(const FunctionCallbackInfo<Value>& args) {
  TextWidget* self = BaseObject::Unwrap<TextWidget>(args.This());
  if (self) {
    Utf8Value text(args.GetIsolate(), args[0]);
    self->set_text(*text);
  }
}

void TextWrappedWidget::Render() {
  ImGui::TextWrapped("%s", text_.c_str());
}

TextDisabledWidget::TextDisabledWidget(Realm* realm, Local<Object> object, const std::string& text)
    : Widget(realm, object), text_(text) {}

void TextDisabledWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  SetProtoProperty(isolate, tmpl, "text", GetText, SetText);

  tmpl->SetClassName(FixedOneByteString(isolate, "TextDisabled"));
  target->Set(FixedOneByteString(isolate, "TextDisabled"), tmpl);
}

void TextDisabledWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Utf8Value text(isolate, args[0]);
  new TextDisabledWidget(env->principal_realm(), args.This(), *text);
}

void TextDisabledWidget::GetText(const FunctionCallbackInfo<Value>& args) {
  TextWidget* self = BaseObject::Unwrap<TextWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->text().c_str()).ToLocalChecked());
  }
}

void TextDisabledWidget::SetText(const FunctionCallbackInfo<Value>& args) {
  TextWidget* self = BaseObject::Unwrap<TextWidget>(args.This());
  if (self) {
    Utf8Value text(args.GetIsolate(), args[0]);
    self->set_text(*text);
  }
}

void TextDisabledWidget::Render() {
  ImGui::TextDisabled("%s", text_.c_str());
}

LabelTextWidget::LabelTextWidget(Realm* realm, Local<Object> object, const std::string& label, const std::string& text)
    : Widget(realm, object, label), text_(text) {}

void LabelTextWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  SetProtoProperty(isolate, tmpl, "text", GetText, SetText);

  tmpl->SetClassName(FixedOneByteString(isolate, "LabelText"));
  target->Set(FixedOneByteString(isolate, "LabelText"), tmpl);
}

void LabelTextWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Utf8Value label(isolate, args[0]);
  Utf8Value text(isolate, args[1]);
  new LabelTextWidget(env->principal_realm(), args.This(), *label, *text);
}

void LabelTextWidget::GetText(const FunctionCallbackInfo<Value>& args) {
  TextWidget* self = BaseObject::Unwrap<TextWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->text().c_str()).ToLocalChecked());
  }
}

void LabelTextWidget::SetText(const FunctionCallbackInfo<Value>& args) {
  TextWidget* self = BaseObject::Unwrap<TextWidget>(args.This());
  if (self) {
    Utf8Value text(args.GetIsolate(), args[0]);
    self->set_text(*text);
  }
}

void LabelTextWidget::Render() {
  EmitEvent("update");
  ImGui::LabelText(label_.c_str(), "%s", text_.c_str());
}

BulletTextWidget::BulletTextWidget(Realm* realm, Local<Object> object, const std::string& text)
    : Widget(realm, object), text_(text) {}

void BulletTextWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  SetProtoProperty(isolate, tmpl, "text", GetText, SetText);

  tmpl->SetClassName(FixedOneByteString(isolate, "BulletText"));
  target->Set(FixedOneByteString(isolate, "BulletText"), tmpl);
}

void BulletTextWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Utf8Value text(isolate, args[0]);
  new BulletTextWidget(env->principal_realm(), args.This(), *text);
}

void BulletTextWidget::GetText(const FunctionCallbackInfo<Value>& args) {
  TextWidget* self = BaseObject::Unwrap<TextWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->text().c_str()).ToLocalChecked());
  }
}

void BulletTextWidget::SetText(const FunctionCallbackInfo<Value>& args) {
  TextWidget* self = BaseObject::Unwrap<TextWidget>(args.This());
  if (self) {
    Utf8Value text(args.GetIsolate(), args[0]);
    self->set_text(*text);
  }
}

void BulletTextWidget::Render() {
  ImGui::BulletText("%s", text_.c_str());
}

SeparatorTextWidget::SeparatorTextWidget(Realm* realm, Local<Object> object, const std::string& text)
    : Widget(realm, object), text_(text) {}

void SeparatorTextWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  SetProtoProperty(isolate, tmpl, "text", GetText, SetText);

  tmpl->SetClassName(FixedOneByteString(isolate, "SeparatorText"));
  target->Set(FixedOneByteString(isolate, "SeparatorText"), tmpl);
}

void SeparatorTextWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Utf8Value text(isolate, args[0]);
  new SeparatorTextWidget(env->principal_realm(), args.This(), *text);
}

void SeparatorTextWidget::GetText(const FunctionCallbackInfo<Value>& args) {
  TextWidget* self = BaseObject::Unwrap<TextWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->text().c_str()).ToLocalChecked());
  }
}

void SeparatorTextWidget::SetText(const FunctionCallbackInfo<Value>& args) {
  TextWidget* self = BaseObject::Unwrap<TextWidget>(args.This());
  if (self) {
    Utf8Value text(args.GetIsolate(), args[0]);
    self->set_text(*text);
  }
}

void SeparatorTextWidget::Render() {
  ImGui::SeparatorText(text_.c_str());
}

}  // namespace nyx
