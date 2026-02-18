#include "nyx/gui/listbox.h"

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

ListBoxWidget::ListBoxWidget(Realm* realm,
                             Local<Object> object,
                             const std::string& label,
                             std::vector<std::string> items,
                             int selected,
                             int height_items)
    : Widget(realm, object,label),
      items_(std::move(items)),
      selected_(selected),
      height_items_(height_items) {}

void ListBoxWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  SetProtoProperty(isolate, tmpl, "selected", GetSelected, SetSelected);
  SetProtoMethod(isolate, tmpl, "setItems", SetItems);

  tmpl->SetClassName(FixedOneByteString(isolate, "ListBox"));
  target->Set(FixedOneByteString(isolate, "ListBox"), tmpl);
}

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

void ListBoxWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value label(isolate, args[0]);
  auto items = ArrayToStringVector(isolate, context, args[1]);
  int selected = args.Length() > 2 ? args[2]->Int32Value(context).FromMaybe(0) : 0;
  int height = args.Length() > 3 ? args[3]->Int32Value(context).FromMaybe(-1) : -1;
  new ListBoxWidget(env->principal_realm(), args.This(), *label, std::move(items), selected, height);
}

void ListBoxWidget::GetSelected(const FunctionCallbackInfo<Value>& args) {
  ListBoxWidget* self = BaseObject::Unwrap<ListBoxWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->selected());
}

void ListBoxWidget::SetSelected(const FunctionCallbackInfo<Value>& args) {
  ListBoxWidget* self = BaseObject::Unwrap<ListBoxWidget>(args.This());
  if (self) {
    Local<Context> ctx = args.GetIsolate()->GetCurrentContext();
    self->set_selected(args[0]->Int32Value(ctx).FromMaybe(0));
  }
}

void ListBoxWidget::SetItems(const FunctionCallbackInfo<Value>& args) {
  ListBoxWidget* self = BaseObject::Unwrap<ListBoxWidget>(args.This());
  if (self) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();
    self->set_items(ArrayToStringVector(isolate, context, args[0]));
  }
}

void ListBoxWidget::Render() {
  // Build const char* array for ImGui
  std::vector<const char*> c_items;
  c_items.reserve(items_.size());
  for (const auto& s : items_) c_items.push_back(s.c_str());

  int old = selected_;
  ImGui::ListBox(label_.c_str(), &selected_, c_items.data(), static_cast<int>(c_items.size()), height_items_);
  if (selected_ != old) {
    Isolate* iso = isolate();
    HandleScope scope(iso);
    EmitEvent("change", Integer::New(iso, selected_));
  }
}

}  // namespace nyx
