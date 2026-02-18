#include "nyx/gui/layout.h"

#include "nyx/env.h"

namespace nyx {
  
using v8::Context;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::ObjectTemplate;
using v8::String;
using v8::Value;

void SeparatorWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  tmpl->SetClassName(FixedOneByteString(isolate, "Separator"));
  target->Set(FixedOneByteString(isolate, "Separator"), tmpl);
}

void SeparatorWidget::New(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args.GetIsolate());
  new SeparatorWidget(env->principal_realm(), args.This());
}

void SeparatorWidget::Render() {
  ImGui::Separator();
}

void SpacingWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  tmpl->SetClassName(FixedOneByteString(isolate, "Spacing"));
  target->Set(FixedOneByteString(isolate, "Spacing"), tmpl);
}

void SpacingWidget::New(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args.GetIsolate());
  new SpacingWidget(env->principal_realm(), args.This());
}

void SpacingWidget::Render() {
  ImGui::Spacing();
}

SameLineWidget::SameLineWidget(Realm* realm, Local<Object> object, float offset, float spacing)
    : Widget(realm, object), offset_(offset), spacing_(spacing) {}

void SameLineWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  tmpl->SetClassName(FixedOneByteString(isolate, "SameLine"));
  target->Set(FixedOneByteString(isolate, "SameLine"), tmpl);
}

void SameLineWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  float offset = args.Length() > 0 ? static_cast<float>(args[0]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  float spacing = args.Length() > 1 ? static_cast<float>(args[1]->NumberValue(context).FromMaybe(-1.0)) : -1.0f;
  new SameLineWidget(env->principal_realm(), args.This(), offset, spacing);
}

void SameLineWidget::Render() {
  ImGui::SameLine(offset_, spacing_);
}

void NewLineWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  tmpl->SetClassName(FixedOneByteString(isolate, "NewLine"));
  target->Set(FixedOneByteString(isolate, "NewLine"), tmpl);
}

void NewLineWidget::New(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args.GetIsolate());
  new NewLineWidget(env->principal_realm(), args.This());
}

void NewLineWidget::Render() {
  ImGui::NewLine();
}

IndentWidget::IndentWidget(Realm* realm, Local<Object> object, float width)
    : Widget(realm, object), width_(width) {}

void IndentWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  tmpl->SetClassName(FixedOneByteString(isolate, "Indent"));
  target->Set(FixedOneByteString(isolate, "Indent"), tmpl);
}

void IndentWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  float width = args.Length() > 0 ? static_cast<float>(args[0]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  new IndentWidget(env->principal_realm(), args.This(), width);
}

void IndentWidget::Render() {
  ImGui::Indent(width_);
  RenderChildren();
  ImGui::Unindent(width_);
}

UnindentWidget::UnindentWidget(Realm* realm, Local<Object> object, float width)
    : Widget(realm, object), width_(width) {}

void UnindentWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  tmpl->SetClassName(FixedOneByteString(isolate, "Unindent"));
  target->Set(FixedOneByteString(isolate, "Unindent"), tmpl);
}

void UnindentWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  float width = args.Length() > 0 ? static_cast<float>(args[0]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  new UnindentWidget(env->principal_realm(), args.This(), width);
}

void UnindentWidget::Render() {
  ImGui::Unindent(width_);
  RenderChildren();
  ImGui::Indent(width_);
}

DummyWidget::DummyWidget(Realm* realm, Local<Object> object, float width, float height)
    : Widget(realm, object), width_(width), height_(height) {}

void DummyWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  tmpl->SetClassName(FixedOneByteString(isolate, "Dummy"));
  target->Set(FixedOneByteString(isolate, "Dummy"), tmpl);
}

void DummyWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  float width = static_cast<float>(args[0]->NumberValue(context).FromMaybe(0.0));
  float height = static_cast<float>(args[1]->NumberValue(context).FromMaybe(0.0));
  new DummyWidget(env->principal_realm(), args.This(), width, height);
}

void DummyWidget::Render() {
  ImGui::Dummy(ImVec2(width_, height_));
}

void GroupWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  tmpl->SetClassName(FixedOneByteString(isolate, "Group"));
  target->Set(FixedOneByteString(isolate, "Group"), tmpl);
}

void GroupWidget::New(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args.GetIsolate());
  new GroupWidget(env->principal_realm(), args.This());
}

void GroupWidget::Render() {
  ImGui::BeginGroup();
  RenderChildren();
  ImGui::EndGroup();
}

DisabledWidget::DisabledWidget(Realm* realm, Local<Object> object, bool disabled)
    : Widget(realm, object), disabled_(disabled) {}

void DisabledWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  SetProtoProperty(isolate, tmpl, "disabled", GetDisabled, SetDisabled);

  tmpl->SetClassName(FixedOneByteString(isolate, "Disabled"));
  target->Set(FixedOneByteString(isolate, "Disabled"), tmpl);
}

void DisabledWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  bool disabled = args.Length() > 0 ? args[0]->BooleanValue(isolate) : true;
  new DisabledWidget(env->principal_realm(), args.This(), disabled);
}

void DisabledWidget::GetDisabled(const FunctionCallbackInfo<Value>& args) {
  DisabledWidget* self = BaseObject::Unwrap<DisabledWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->disabled());
}

void DisabledWidget::SetDisabled(const FunctionCallbackInfo<Value>& args) {
  DisabledWidget* self = BaseObject::Unwrap<DisabledWidget>(args.This());
  if (self) self->set_disabled(args[0]->BooleanValue(args.GetIsolate()));
}

void DisabledWidget::Render() {
  ImGui::BeginDisabled(disabled_);
  RenderChildren();
  ImGui::EndDisabled();
}

}  // namespace nyx
