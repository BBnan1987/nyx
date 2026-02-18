#include "nyx/gui/popups.h"

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

PopupWidget::PopupWidget(Realm* realm, Local<Object> object, const std::string& id)
    : Widget(realm, object), id_(id) {}

void PopupWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  SetProtoMethod(isolate, tmpl, "open", Open);
  SetProtoMethod(isolate, tmpl, "close", Close);

  tmpl->SetClassName(FixedOneByteString(isolate, "Popup"));
  target->Set(FixedOneByteString(isolate, "Popup"), tmpl);
}

void PopupWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Utf8Value id(isolate, args[0]);
  new PopupWidget(env->principal_realm(), args.This(), *id);
}

void PopupWidget::Open(const FunctionCallbackInfo<Value>& args) {
  PopupWidget* self = BaseObject::Unwrap<PopupWidget>(args.This());
  if (self) self->DoOpen();
}

void PopupWidget::Close(const FunctionCallbackInfo<Value>& args) {
  PopupWidget* self = BaseObject::Unwrap<PopupWidget>(args.This());
  if (self) self->DoClose();
}

void PopupWidget::Render() {
  if (should_open_) {
    ImGui::OpenPopup(id_.c_str());
    should_open_ = false;
  }
  if (ImGui::BeginPopup(id_.c_str())) {
    RenderChildren();
    if (should_close_) {
      ImGui::CloseCurrentPopup();
      should_close_ = false;
    }
    ImGui::EndPopup();
  }
}

ModalWidget::ModalWidget(Realm* realm, Local<Object> object, const std::string& title)
    : Widget(realm, object), title_(title) {}

void ModalWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  SetProtoMethod(isolate, tmpl, "open", Open);
  SetProtoMethod(isolate, tmpl, "close", Close);
  SetProtoProperty(isolate, tmpl, "isOpen", IsOpen);

  tmpl->SetClassName(FixedOneByteString(isolate, "Modal"));
  target->Set(FixedOneByteString(isolate, "Modal"), tmpl);
}

void ModalWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Utf8Value title(isolate, args[0]);
  new ModalWidget(env->principal_realm(), args.This(), *title);
}

void ModalWidget::Open(const FunctionCallbackInfo<Value>& args) {
  ModalWidget* self = BaseObject::Unwrap<ModalWidget>(args.This());
  if (self) self->DoOpen();
}

void ModalWidget::Close(const FunctionCallbackInfo<Value>& args) {
  ModalWidget* self = BaseObject::Unwrap<ModalWidget>(args.This());
  if (self) self->DoClose();
}

void ModalWidget::IsOpen(const FunctionCallbackInfo<Value>& args) {
  ModalWidget* self = BaseObject::Unwrap<ModalWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->is_open());
}

void ModalWidget::Render() {
  if (should_open_) {
    ImGui::OpenPopup(title_.c_str());
    should_open_ = false;
  }
  is_open_ = ImGui::BeginPopupModal(title_.c_str(), nullptr);
  if (is_open_) {
    RenderChildren();
    if (should_close_) {
      ImGui::CloseCurrentPopup();
      should_close_ = false;
    }
    ImGui::EndPopup();
  }
}

}  // namespace nyx
