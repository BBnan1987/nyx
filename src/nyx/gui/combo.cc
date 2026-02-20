#include "nyx/gui/combo.h"

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
using v8::Object;
using v8::ObjectTemplate;
using v8::String;
using v8::Value;

ComboWidget::ComboWidget(Realm* realm,
                         Local<Object> object,
                         const std::string& label,
                         std::vector<std::string> items,
                         int selected,
                         ImGuiComboFlags flags)
    : Widget(realm, object, label), items_(std::move(items)), selected_(selected), flags_(flags) {}

void ComboWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  SetProtoProperty(isolate, tmpl, "selected", SelectedGetter, SelectedSetter);
  SetProtoProperty(isolate, tmpl, "items", ItemsGetter, ItemsSetter);

  tmpl->SetClassName(FixedOneByteString(isolate, "Combo"));
  target->Set(FixedOneByteString(isolate, "Combo"), tmpl);
}

// fixme: could be an overload of FromV8Value(context, js_array, std::vector<T>& out)
static std::vector<std::string> ArrayToStringVector(Isolate* isolate, Local<Context> context, Local<Value> val) {
  std::vector<std::string> result;
  if (!val->IsArray()) return result;
  Local<Array> arr = val.As<Array>();
  for (uint32_t i = 0; i < arr->Length(); i++) {
    Local<Value> elem;
    if (arr->Get(context, i).ToLocal(&elem)) {
      Utf8Value str(isolate, elem);
      result.push_back(*str);
    }
  }
  return result;
}

void ComboWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value label(isolate, args[0]);
  auto items = ArrayToStringVector(isolate, context, args[1]);
  int selected = args[2]->Int32Value(context).FromMaybe(0);
  ImGuiComboFlags flags = args[3]->Uint32Value(context).FromMaybe(ImGuiComboFlags_None);
  new ComboWidget(env->principal_realm(), args.This(), *label, std::move(items), selected);
}

void ComboWidget::FlagsGetter(const v8::FunctionCallbackInfo<v8::Value>& args) {
  ComboWidget* self = BaseObject::Unwrap<ComboWidget>(args.This());
  if (self) args.GetReturnValue().Set(static_cast<uint32_t>(self->flags_));
}

void ComboWidget::FlagsSetter(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Local<Context> context = env->context();
  ComboWidget* self = BaseObject::Unwrap<ComboWidget>(args.This());
  if (self && args[0]->IsNumber()) {
    self->flags_ = args[0]->Uint32Value(context).FromMaybe(0);
  }
}

void ComboWidget::SelectedGetter(const FunctionCallbackInfo<Value>& args) {
  ComboWidget* self = BaseObject::Unwrap<ComboWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->selected_);
}

void ComboWidget::SelectedSetter(const FunctionCallbackInfo<Value>& args) {
  ComboWidget* self = BaseObject::Unwrap<ComboWidget>(args.This());
  if (self) {
    Local<Context> ctx = args.GetIsolate()->GetCurrentContext();
    self->selected_ = args[0]->Int32Value(ctx).FromMaybe(0);
  }
}

void ComboWidget::ItemsGetter(const v8::FunctionCallbackInfo<v8::Value>& args) {
  ComboWidget* self = BaseObject::Unwrap<ComboWidget>(args.This());
  if (self) {
    Local<Context> context = args.GetIsolate()->GetCurrentContext();
    Local<Value> items;
    if (ToV8Value(context, self->selected_).ToLocal(&items)) {
      args.GetReturnValue().Set(items);
    }
  }
}

void ComboWidget::ItemsSetter(const FunctionCallbackInfo<Value>& args) {
  ComboWidget* self = BaseObject::Unwrap<ComboWidget>(args.This());
  if (self) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();
    self->items_ = std::move(ArrayToStringVector(isolate, context, args[0]));
  }
}

void ComboWidget::Render() {
  const char* preview =
      (selected_ >= 0 && selected_ < static_cast<int>(items_.size())) ? items_[selected_].c_str() : "";
  if (ImGui::BeginCombo(label_.c_str(), preview, flags_)) {
    for (int i = 0; i < static_cast<int>(items_.size()); i++) {
      bool is_selected = (i == selected_);
      if (ImGui::Selectable(items_[i].c_str(), is_selected)) {
        if (selected_ != i) {
          selected_ = i;
          Isolate* iso = isolate();
          HandleScope scope(iso);
          EmitEvent("change", Integer::New(iso, selected_));
        }
      }
      if (is_selected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }
}

}  // namespace nyx
