#include "nyx/gui/tables.h"

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

TableWidget::TableWidget(
    Realm* realm, Local<Object> object, const std::string& id, int columns, ImGuiTableFlags flags)
    : Widget(realm, object), id_(id), columns_(columns), flags_(flags) {}

void TableWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  SetProtoMethod(isolate, tmpl, "addColumn", AddColumn);

  tmpl->SetClassName(FixedOneByteString(isolate, "Table"));
  target->Set(FixedOneByteString(isolate, "Table"), tmpl);
}

void TableWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value id(isolate, args[0]);
  int columns = args[1]->Int32Value(context).FromMaybe(1);
  ImGuiTableFlags flags =
      args.Length() > 2 ? static_cast<ImGuiTableFlags>(args[2]->Uint32Value(context).FromMaybe(0)) : 0;
  new TableWidget(env->principal_realm(), args.This(), *id, columns, flags);
}

void TableWidget::AddColumn(const FunctionCallbackInfo<Value>& args) {
  TableWidget* self = BaseObject::Unwrap<TableWidget>(args.This());
  if (self) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();
    Utf8Value name(isolate, args[0]);
    ImGuiTableColumnFlags flags =
        args.Length() > 1 ? static_cast<ImGuiTableColumnFlags>(args[1]->Uint32Value(context).FromMaybe(0)) : 0;
    float width = args.Length() > 2 ? static_cast<float>(args[2]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
    self->_AddColumn(*name, flags, width);
  }
}

void TableWidget::Render() {
  if (ImGui::BeginTable(id_.c_str(), columns_, flags_)) {
    // Setup columns
    for (const auto& col : column_defs_) {
      ImGui::TableSetupColumn(col.name.c_str(), col.flags, col.width);
    }
    if (!column_defs_.empty()) {
      ImGui::TableHeadersRow();
    }
    RenderChildren();
    ImGui::EndTable();
  }
}

void TableWidget::_AddColumn(const std::string& name, ImGuiTableColumnFlags flags, float width) {
  column_defs_.push_back({name, flags, width});
}

void TableRowWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  tmpl->SetClassName(FixedOneByteString(isolate, "TableRow"));
  target->Set(FixedOneByteString(isolate, "TableRow"), tmpl);
}

void TableRowWidget::New(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args.GetIsolate());
  new TableRowWidget(env->principal_realm(), args.This());
}

void TableRowWidget::Render() {
  ImGui::TableNextRow();
  // Each child occupies the next column
  auto snapshot = children_;
  for (size_t i = 0; i < snapshot.size(); i++) {
    ImGui::TableSetColumnIndex(static_cast<int>(i));
    if (snapshot[i]->visible()) {
      snapshot[i]->Render();
    }
  }
}

}  // namespace nyx
