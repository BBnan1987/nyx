#include "nyx/gui/trees.h"

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

TreeNodeWidget::TreeNodeWidget(Realm* realm, Local<Object> object, const std::string& label)
    : Widget(realm, object, label) {}

void TreeNodeWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  SetProtoProperty(isolate, tmpl, "open", GetOpen);

  tmpl->SetClassName(FixedOneByteString(isolate, "TreeNode"));
  target->Set(FixedOneByteString(isolate, "TreeNode"), tmpl);
}

void TreeNodeWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Utf8Value label(isolate, args[0]);
  new TreeNodeWidget(env->principal_realm(), args.This(), *label);
}

void TreeNodeWidget::GetOpen(const FunctionCallbackInfo<Value>& args) {
  TreeNodeWidget* self = BaseObject::Unwrap<TreeNodeWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->open());
}

void TreeNodeWidget::Render() {
  open_ = ImGui::TreeNode(label_.c_str());
  if (open_) {
    RenderChildren();
    ImGui::TreePop();
  }
}

CollapsingHeaderWidget::CollapsingHeaderWidget(Realm* realm,
                                               Local<Object> object,
                                               const std::string& label,
                                               ImGuiTreeNodeFlags flags)
    : Widget(realm, object, label), flags_(flags) {}

void CollapsingHeaderWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  SetProtoProperty(isolate, tmpl, "open", GetOpen);

  tmpl->SetClassName(FixedOneByteString(isolate, "CollapsingHeader"));
  target->Set(FixedOneByteString(isolate, "CollapsingHeader"), tmpl);
}

void CollapsingHeaderWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value label(isolate, args[0]);
  ImGuiTreeNodeFlags flags =
      args.Length() > 1 ? static_cast<ImGuiTreeNodeFlags>(args[1]->Uint32Value(context).FromMaybe(0)) : 0;
  new CollapsingHeaderWidget(env->principal_realm(), args.This(), *label, flags);
}

void CollapsingHeaderWidget::GetOpen(const FunctionCallbackInfo<Value>& args) {
  CollapsingHeaderWidget* self = BaseObject::Unwrap<CollapsingHeaderWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->open());
}

void CollapsingHeaderWidget::Render() {
  open_ = ImGui::CollapsingHeader(label_.c_str(), flags_);
  if (open_) {
    RenderChildren();
  }
}

}  // namespace nyx
