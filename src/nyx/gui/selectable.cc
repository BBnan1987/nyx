#include "nyx/gui/selectable.h"

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

SelectableWidget::SelectableWidget(Realm* realm, Local<Object> object, const std::string& label, bool selected)
    : Widget(realm, object, label), selected_(selected) {}

void SelectableWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  SetProtoProperty(isolate, tmpl, "selected", GetSelected, SetSelected);

  tmpl->SetClassName(FixedOneByteString(isolate, "Selectable"));
  target->Set(FixedOneByteString(isolate, "Selectable"), tmpl);
}

void SelectableWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Utf8Value label(isolate, args[0]);
  bool selected = args.Length() > 1 ? args[1]->BooleanValue(isolate) : false;
  new SelectableWidget(env->principal_realm(), args.This(), *label, selected);
}

void SelectableWidget::GetSelected(const FunctionCallbackInfo<Value>& args) {
  SelectableWidget* self = BaseObject::Unwrap<SelectableWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->selected());
}

void SelectableWidget::SetSelected(const FunctionCallbackInfo<Value>& args) {
  SelectableWidget* self = BaseObject::Unwrap<SelectableWidget>(args.This());
  if (self) self->set_selected(args[0]->BooleanValue(args.GetIsolate()));
}

void SelectableWidget::Render() {
  bool old = selected_;
  ImGui::Selectable(label_.c_str(), &selected_);
  if (selected_ != old) {
    Isolate* iso = isolate();
    HandleScope scope(iso);
    EmitEvent("change", Boolean::New(iso, selected_));
  }
}

}  // namespace nyx
