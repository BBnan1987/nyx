#include "nyx/gui/colors.h"

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

ColorWidget::ColorWidget(Realm* realm,
                         Local<Object> object,
                         const std::string& label,
                         ColorWidgetType type,
                         ImGuiColorEditFlags flags,
                         ImVec4 color,
                         ImVec4 ref_color,
                         ImVec2 size)
    : Widget(realm, object, label), type_(type), flags_(flags), color_(color), ref_color_(ref_color), size_(size) {}

void ColorWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  SetProtoProperty(isolate, tmpl, "color", ColorGetter, ColorSetter);

  tmpl->SetClassName(FixedOneByteString(isolate, "ColorWidget"));
  target->Set(FixedOneByteString(isolate, "ColorWidget"), tmpl);
}

void ColorWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Local<Context> context = env->context();
  CHECK(args[0]->IsString());
  CHECK(args[1]->IsNumber());
  CHECK(args[2]->IsNumber());
  CHECK(args[3]->IsObject() || args[3]->IsArray());
  CHECK(args[4]->IsObject() || args[4]->IsArray());
  CHECK(args[5]->IsObject() || args[5]->IsArray());
  Utf8Value label(isolate, args[0]);
  ColorWidgetType type = static_cast<ColorWidgetType>(args[1]->NumberValue(context).FromJust());
  ImGuiColorEditFlags flags = static_cast<ImGuiColorEditFlags>(args[2]->NumberValue(context).FromJust());
  ImVec4 color = ImVec4::FromObject(isolate, args[3].As<Object>());
  ImVec4 ref_color = ImVec4::FromObject(isolate, args[4].As<Object>());
  ImVec2 size = ImVec2::FromObject(isolate, args[5].As<Object>());
  new ColorWidget(env->principal_realm(), args.This(), *label, type, flags, color, ref_color, size);
}

void ColorWidget::FlagsGetter(const v8::FunctionCallbackInfo<v8::Value>& args) {
  ColorWidget* self = BaseObject::Unwrap<ColorWidget>(args.This());
  if (self) args.GetReturnValue().Set(static_cast<uint32_t>(self->flags_));
}

void ColorWidget::FlagsSetter(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Local<Context> context = env->context();
  ColorWidget* self = BaseObject::Unwrap<ColorWidget>(args.This());
  if (self && args[0]->IsNumber()) {
    self->flags_ = args[0]->Uint32Value(context).FromMaybe(0);
  }
}

void ColorWidget::ColorGetter(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Local<Context> context = env->context();
  ColorWidget* self = BaseObject::Unwrap<ColorWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->color_.ToObject(context));
}

void ColorWidget::ColorSetter(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  ColorWidget* self = BaseObject::Unwrap<ColorWidget>(args.This());
  if (self) {
    self->color_ = ImVec4::FromObject(isolate, args[0]);
  }
}

void ColorWidget::RefColorGetter(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Local<Context> context = env->context();
  ColorWidget* self = BaseObject::Unwrap<ColorWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->ref_color_.ToObject(context));
}

void ColorWidget::RefColorSetter(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  ColorWidget* self = BaseObject::Unwrap<ColorWidget>(args.This());
  if (self) {
    self->ref_color_ = ImVec4::FromObject(isolate, args[0]);
  }
}

void ColorWidget::SizeGetter(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Local<Context> context = env->context();
  ColorWidget* self = BaseObject::Unwrap<ColorWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->size_.ToObject(context));
}

void ColorWidget::SizeSetter(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  ColorWidget* self = BaseObject::Unwrap<ColorWidget>(args.This());
  if (self) {
    self->size_ = ImVec2::FromObject(isolate, args[0]);
  }
}

void ColorWidget::Render() {
  bool change = false;
  switch (type_) {
    case ColorEdit3:
      change = ImGui::ColorEdit3(label_.c_str(), &color_.x,flags_);
      break;
    case ColorEdit4:
      change = ImGui::ColorEdit4(label_.c_str(), &color_.x, flags_);
      break;
    case ColorPicker3:
      change = ImGui::ColorPicker3(label_.c_str(), &color_.x, flags_);
      break;
    case ColorPicker4:
      change = ImGui::ColorPicker4(label_.c_str(), &color_.x, flags_, &ref_color_.x);
      break;
    case ColorButton:
      if (ImGui::ColorButton(label_.c_str(), color_, flags_, size_)) {
        EmitEvent("click");
      }
      break;
  }
  if (change) {
    EmitEvent("change");
  }
}

}  // namespace nyx
