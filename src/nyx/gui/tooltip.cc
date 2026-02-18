#include "nyx/gui/tooltip.h"

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

TooltipWidget::TooltipWidget(Realm* realm, Local<Object> object, const std::string& text)
    : Widget(realm, object), text_(text) {}

void TooltipWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  SetProtoProperty(isolate, tmpl, "text", GetText, SetText);

  tmpl->SetClassName(FixedOneByteString(isolate, "Tooltip"));
  target->Set(FixedOneByteString(isolate, "Tooltip"), tmpl);
}

void TooltipWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  std::string text;
  if (args.Length() > 0 && !args[0]->IsUndefined()) {
    Utf8Value t(isolate, args[0]);
    text = *t;
  }
  new TooltipWidget(env->principal_realm(), args.This(), text);
}

void TooltipWidget::GetText(const FunctionCallbackInfo<Value>& args) {
  TooltipWidget* self = BaseObject::Unwrap<TooltipWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->text().c_str()).ToLocalChecked());
  }
}

void TooltipWidget::SetText(const FunctionCallbackInfo<Value>& args) {
  TooltipWidget* self = BaseObject::Unwrap<TooltipWidget>(args.This());
  if (self) {
    Utf8Value text(args.GetIsolate(), args[0]);
    self->set_text(*text);
  }
}

void TooltipWidget::Render() {
  if (ImGui::IsItemHovered()) {
    ImGui::BeginTooltip();
    if (!text_.empty()) {
      ImGui::TextUnformatted(text_.c_str());
    }
    RenderChildren();
    ImGui::EndTooltip();
  }
}

}  // namespace nyx
