#include "nyx/gui/input.h"

#include "nyx/env.h"

namespace nyx {

using v8::Array;
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

InputTextWidget::InputTextWidget(Realm* realm,
                                 Local<Object> object,
                                 const std::string& label,
                                 const std::string& text,
                                 size_t max_length,
                                 ImGuiInputTextFlags flags)
    : Widget(realm, object, label), text_(text), max_length_(max_length), flags_(flags), multiline_(false) {}

void InputTextWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  SetProtoProperty(isolate, tmpl, "text", TextGetter, TextSetter);
  SetProtoProperty(isolate, tmpl, "maxLength", MaxLengthGetter, MaxLengthSetter);
  SetProtoProperty(isolate, tmpl, "flags", FlagsGetter, FlagsSetter);
  SetProtoProperty(isolate, tmpl, "hint", HintGetter, HintSetter);
  SetProtoProperty(isolate, tmpl, "multiLine", MultilineGetter, MultilineSetter);

  tmpl->SetClassName(FixedOneByteString(isolate, "InputTextWidget"));
  target->Set(FixedOneByteString(isolate, "InputTextWidget"), tmpl);
}

void InputTextWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value label(isolate, args[0]);
  Utf8Value text(isolate, args[1]);
  size_t max_length = static_cast<size_t>(args[2]->Uint32Value(context).FromMaybe(256));
  ImGuiInputTextFlags flags =
      static_cast<ImGuiInputTextFlags>(args[3]->Uint32Value(context).FromMaybe(ImGuiInputTextFlags_None));
  new InputTextWidget(env->principal_realm(), args.This(), *label, *text, max_length, flags);
}

void InputTextWidget::TextGetter(const FunctionCallbackInfo<Value>& args) {
  InputTextWidget* self = BaseObject::Unwrap<InputTextWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(OneByteString(self->isolate(), self->text_));
  }
}

void InputTextWidget::TextSetter(const FunctionCallbackInfo<Value>& args) {
  InputTextWidget* self = BaseObject::Unwrap<InputTextWidget>(args.This());
  if (self) {
    Utf8Value text(args.GetIsolate(), args[0]);
    self->text_ = *text;
  }
}

void InputTextWidget::MaxLengthGetter(const v8::FunctionCallbackInfo<v8::Value>& args) {
  InputTextWidget* self = BaseObject::Unwrap<InputTextWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(self->max_length_);
  }
}

void InputTextWidget::MaxLengthSetter(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Local<Context> context = env->context();
  InputTextWidget* self = BaseObject::Unwrap<InputTextWidget>(args.This());
  if (self) {
    self->max_length_ = static_cast<size_t>(args[0]->NumberValue(context).FromMaybe(256));
  }
}

void InputTextWidget::FlagsGetter(const FunctionCallbackInfo<Value>& args) {
  InputTextWidget* self = BaseObject::Unwrap<InputTextWidget>(args.This());
  if (self) args.GetReturnValue().Set(static_cast<uint32_t>(self->flags_));
}

void InputTextWidget::FlagsSetter(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Local<Context> context = env->context();
  InputTextWidget* self = BaseObject::Unwrap<InputTextWidget>(args.This());
  if (self && args[0]->IsNumber()) {
    self->flags_ = args[0]->Uint32Value(context).FromMaybe(0);
  }
}

void InputTextWidget::HintGetter(const FunctionCallbackInfo<Value>& args) {
  InputTextWidget* self = BaseObject::Unwrap<InputTextWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(OneByteString(self->isolate(), self->hint_));
  }
}

void InputTextWidget::HintSetter(const FunctionCallbackInfo<Value>& args) {
  InputTextWidget* self = BaseObject::Unwrap<InputTextWidget>(args.This());
  if (self) {
    Utf8Value text(args.GetIsolate(), args[0]);
    self->hint_ = *text;
  }
}

void InputTextWidget::MultilineGetter(const v8::FunctionCallbackInfo<v8::Value>& args) {
  InputTextWidget* self = BaseObject::Unwrap<InputTextWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(self->multiline_);
  }
}

void InputTextWidget::MultilineSetter(const v8::FunctionCallbackInfo<v8::Value>& args) {
  InputTextWidget* self = BaseObject::Unwrap<InputTextWidget>(args.This());
  if (self && args[0]->IsBoolean()) {
    self->multiline_ = args[0]->BooleanValue(args.GetIsolate());
  }
}

void InputTextWidget::Render() {
  std::vector<char> buf(max_length_ + 1, '\0');
  std::copy(text_.begin(), text_.end(), buf.begin());
  std::string new_text;

  if (multiline_) {
    if (ImGui::InputTextMultiline(label_.c_str(), buf.data(), max_length_ + 1, {}, flags_)) {
      // todo: use result of call?
    }
  } else if (!hint_.empty()) {
    if (ImGui::InputTextWithHint(label_.c_str(), hint_.c_str(), buf.data(), max_length_ + 1, flags_)) {
      // todo: use result of call?
    }
  } else {
    if (ImGui::InputText(label_.c_str(), buf.data(), max_length_ + 1, flags_)) {
      // todo: use result of call?
    }
  }

  new_text = buf.data();
  if (new_text != text_) {
    text_ = std::move(new_text);
    Isolate* iso = isolate();
    HandleScope scope(iso);
    EmitEvent("change", String::NewFromUtf8(iso, text_.c_str()).ToLocalChecked());
  }
}

InputNumberWidget::InputNumberWidget(Realm* realm,
                                     Local<Object> object,
                                     const std::string& label,
                                     InputNumberValueType type,
                                     int components,
                                     const std::array<double, 4>& values,
                                     double step,
                                     double step_fast,
                                     const std::string& format,
                                     ImGuiInputTextFlags flags)
    : Widget(realm, object, label),
      format_(format),
      type_(type),
      components_(components),
      flags_(flags),
      values_(values),
      step_(step),
      step_fast_(step_fast) {}

void InputNumberWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  SetProtoProperty(isolate, tmpl, "format", FormatGetter, FormatSetter);
  SetProtoProperty(isolate, tmpl, "flags", FlagsGetter, FlagsSetter);
  SetProtoProperty(isolate, tmpl, "value", ValueGetter, ValueSetter);
  SetProtoProperty(isolate, tmpl, "step", StepGetter, StepSetter);
  SetProtoProperty(isolate, tmpl, "stepFast", StepFastGetter, StepFastSetter);

  tmpl->SetClassName(FixedOneByteString(isolate, "InputNumberWidget"));
  target->Set(FixedOneByteString(isolate, "InputNumberWidget"), tmpl);
}

void InputNumberWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value label(isolate, args[0]);
  int type_int = args[1]->Int32Value(context).FromMaybe(0);
  InputNumberValueType type = type_int == 1 ? Int : type_int == 2 ? Double : Float;
  int components = args[2]->Int32Value(context).FromMaybe(1);
  if (components < 1) components = 1;
  if (components > 4) components = 4;
  std::array<double, 4> values = {};
  if (args[3]->IsArray()) {
    Local<Array> arr = args[3].As<Array>();
    for (int i = 0; i < components; i++) {
      Local<Value> v = arr->Get(context, i).ToLocalChecked();
      values[i] = type == Int ? v->Int32Value(context).FromMaybe(0) : v->NumberValue(context).FromMaybe(0.0);
    }
  }
  double step = type == Int ? args[4]->Int32Value(context).FromMaybe(kDefaultIntStep)
                            : args[4]->NumberValue(context).FromMaybe(kDefaultFloatStep);
  double step_fast = type == Int ? args[5]->Int32Value(context).FromMaybe(kDefaultIntStepFast)
                                 : args[5]->NumberValue(context).FromMaybe(kDefaultFloatStepFast);
  Utf8Value format(isolate, args[6]);
  ImGuiInputTextFlags flags =
      static_cast<ImGuiInputTextFlags>(args[7]->Uint32Value(context).FromMaybe(ImGuiInputTextFlags_None));
  new InputNumberWidget(
      env->principal_realm(), args.This(), *label, type, components, values, step, step_fast, *format, flags);
}

void InputNumberWidget::FormatGetter(const FunctionCallbackInfo<Value>& args) {
  InputNumberWidget* self = BaseObject::Unwrap<InputNumberWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(OneByteString(self->isolate(), self->format_));
  }
}

void InputNumberWidget::FormatSetter(const FunctionCallbackInfo<Value>& args) {
  InputNumberWidget* self = BaseObject::Unwrap<InputNumberWidget>(args.This());
  if (self) {
    Utf8Value text(args.GetIsolate(), args[0]);
    self->format_ = *text;
  }
}

void InputNumberWidget::FlagsGetter(const FunctionCallbackInfo<Value>& args) {
  InputNumberWidget* self = BaseObject::Unwrap<InputNumberWidget>(args.This());
  if (self) args.GetReturnValue().Set(static_cast<uint32_t>(self->flags_));
}

void InputNumberWidget::FlagsSetter(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Local<Context> context = env->context();
  InputNumberWidget* self = BaseObject::Unwrap<InputNumberWidget>(args.This());
  if (self && args[0]->IsNumber()) {
    self->flags_ = args[0]->Uint32Value(context).FromMaybe(0);
  }
}

void InputNumberWidget::ValueGetter(const FunctionCallbackInfo<Value>& args) {
  InputNumberWidget* self = BaseObject::Unwrap<InputNumberWidget>(args.This());
  if (!self) return;
  Isolate* isolate = args.GetIsolate();
  if (self->components_ == 1) {
    if (self->type_ == Float) {
      args.GetReturnValue().Set(static_cast<float>(self->values_[0]));
    } else if (self->type_ == Int) {
      args.GetReturnValue().Set(static_cast<int32_t>(self->values_[0]));
    } else {
      args.GetReturnValue().Set(self->values_[0]);
    }
  } else {
    Local<Context> context = isolate->GetCurrentContext();
    Local<Array> arr = Array::New(isolate, self->components_);
    for (int i = 0; i < self->components_; i++) {
      Local<Value> v;
      if (self->type_ == Int) {
        v = Integer::New(isolate, static_cast<int32_t>(self->values_[i])).As<Value>();
      } else if (self->type_ == Float) {
        v = Number::New(isolate, static_cast<double>(static_cast<float>(self->values_[i]))).As<Value>();
      } else {
        v = Number::New(isolate, self->values_[i]).As<Value>();
      }
      arr->Set(context, i, v).Check();
    }
    args.GetReturnValue().Set(arr);
  }
}

void InputNumberWidget::ValueSetter(const FunctionCallbackInfo<Value>& args) {
  InputNumberWidget* self = BaseObject::Unwrap<InputNumberWidget>(args.This());
  if (!self) return;
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  if (self->components_ == 1) {
    self->values_[0] = self->type_ == Int ? args[0]->Int32Value(context).FromMaybe(kDefaultIntValue)
                                          : args[0]->NumberValue(context).FromMaybe(kDefaultFloatValue);
  } else if (args[0]->IsArray()) {
    Local<Array> arr = args[0].As<Array>();
    for (int i = 0; i < self->components_; i++) {
      Local<Value> v = arr->Get(context, i).ToLocalChecked();
      self->values_[i] =
          self->type_ == Int ? v->Int32Value(context).FromMaybe(0) : v->NumberValue(context).FromMaybe(0.0);
    }
  }
}

void InputNumberWidget::StepGetter(const FunctionCallbackInfo<Value>& args) {
  InputNumberWidget* self = BaseObject::Unwrap<InputNumberWidget>(args.This());
  if (self) {
    if (self->type_ == Float) {
      args.GetReturnValue().Set(static_cast<float>(self->step_));
    } else if (self->type_ == Int) {
      args.GetReturnValue().Set(static_cast<int>(self->step_));
    } else {
      args.GetReturnValue().Set(self->step_);
    }
  }
}

void InputNumberWidget::StepSetter(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Local<Context> context = env->context();
  InputNumberWidget* self = BaseObject::Unwrap<InputNumberWidget>(args.This());
  if (self) {
    self->step_ = self->type_ == Int ? args[0]->Int32Value(context).FromMaybe(kDefaultIntStep)
                                     : args[0]->NumberValue(context).FromMaybe(kDefaultFloatStep);
  }
}

void InputNumberWidget::StepFastGetter(const FunctionCallbackInfo<Value>& args) {
  InputNumberWidget* self = BaseObject::Unwrap<InputNumberWidget>(args.This());
  if (self) {
    if (self->type_ == Float) {
      args.GetReturnValue().Set(static_cast<float>(self->step_fast_));
    } else if (self->type_ == Int) {
      args.GetReturnValue().Set(static_cast<int>(self->step_fast_));
    } else {
      args.GetReturnValue().Set(self->step_fast_);
    }
  }
}

void InputNumberWidget::StepFastSetter(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Local<Context> context = env->context();
  InputNumberWidget* self = BaseObject::Unwrap<InputNumberWidget>(args.This());
  if (self) {
    self->step_fast_ = self->type_ == Int ? args[0]->Int32Value(context).FromMaybe(kDefaultIntStepFast)
                                          : args[0]->NumberValue(context).FromMaybe(kDefaultFloatStepFast);
  }
}

void InputNumberWidget::Render() {
  std::array<double, 4> old_values = values_;
  if (type_ == Float) {
    float fvals[4];
    float fstep = static_cast<float>(step_);
    float fstep_fast = static_cast<float>(step_fast_);
    for (int i = 0; i < components_; i++) fvals[i] = static_cast<float>(values_[i]);
    ImGui::InputScalarN(label_.c_str(),
                        ImGuiDataType_Float,
                        fvals,
                        components_,
                        fstep > 0.0f ? &fstep : nullptr,
                        fstep_fast > 0.0f ? &fstep_fast : nullptr,
                        format_.c_str(),
                        flags_);
    for (int i = 0; i < components_; i++) values_[i] = static_cast<double>(fvals[i]);
  } else if (type_ == Int) {
    int32_t ivals[4];
    int32_t istep = static_cast<int32_t>(step_);
    int32_t istep_fast = static_cast<int32_t>(step_fast_);
    for (int i = 0; i < components_; i++) ivals[i] = static_cast<int32_t>(values_[i]);
    ImGui::InputScalarN(label_.c_str(),
                        ImGuiDataType_S32,
                        ivals,
                        components_,
                        istep > 0 ? &istep : nullptr,
                        istep_fast > 0 ? &istep_fast : nullptr,
                        format_.c_str(),
                        flags_);
    for (int i = 0; i < components_; i++) values_[i] = static_cast<double>(ivals[i]);
  } else {  // Double
    double dvals[4];
    double dstep = step_;
    double dstep_fast = step_fast_;
    for (int i = 0; i < components_; i++) dvals[i] = values_[i];
    ImGui::InputScalarN(label_.c_str(),
                        ImGuiDataType_Double,
                        dvals,
                        components_,
                        dstep > 0.0 ? &dstep : nullptr,
                        dstep_fast > 0.0 ? &dstep_fast : nullptr,
                        format_.c_str(),
                        flags_);
    for (int i = 0; i < components_; i++) values_[i] = dvals[i];
  }
  bool changed = false;
  for (int i = 0; i < components_; i++) {
    if (values_[i] != old_values[i]) {
      changed = true;
      break;
    }
  }
  if (changed) {
    Isolate* iso = isolate();
    HandleScope scope(iso);
    if (components_ == 1) {
      EmitEvent("change", Number::New(iso, values_[0]));
    } else {
      Local<Context> ctx = iso->GetCurrentContext();
      Local<Array> arr = Array::New(iso, components_);
      for (int i = 0; i < components_; i++) arr->Set(ctx, i, Number::New(iso, values_[i])).Check();
      EmitEvent("change", arr);
    }
  }
}

}  // namespace nyx
