#include "nyx/gui/tabs.h"

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

TabBarWidget::TabBarWidget(Realm* realm, Local<Object> object, const std::string& id)
    : Widget(realm, object), id_(id) {}

void TabBarWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  tmpl->SetClassName(FixedOneByteString(isolate, "TabBar"));
  target->Set(FixedOneByteString(isolate, "TabBar"), tmpl);
}

void TabBarWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Utf8Value id(isolate, args[0]);
  new TabBarWidget(env->principal_realm(), args.This(), *id);
}

void TabBarWidget::Render() {
  if (ImGui::BeginTabBar(id_.c_str())) {
    RenderChildren();
    ImGui::EndTabBar();
  }
}

TabItemWidget::TabItemWidget(Realm* realm, Local<Object> object, const std::string& label)
    : Widget(realm, object, label) {}

void TabItemWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  SetProtoProperty(isolate, tmpl, "selected", GetSelected);

  tmpl->SetClassName(FixedOneByteString(isolate, "TabItem"));
  target->Set(FixedOneByteString(isolate, "TabItem"), tmpl);
}

void TabItemWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Utf8Value label(isolate, args[0]);
  new TabItemWidget(env->principal_realm(), args.This(), *label);
}

void TabItemWidget::GetSelected(const FunctionCallbackInfo<Value>& args) {
  TabItemWidget* self = BaseObject::Unwrap<TabItemWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->selected());
}

void TabItemWidget::Render() {
  selected_ = ImGui::BeginTabItem(label_.c_str());
  if (selected_) {
    RenderChildren();
    ImGui::EndTabItem();
  }
}

}  // namespace nyx
