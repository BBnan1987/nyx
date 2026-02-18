#include "nyx/gui/menus.h"

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

void MainMenuBarWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  tmpl->SetClassName(FixedOneByteString(isolate, "MainMenuBar"));
  target->Set(FixedOneByteString(isolate, "MainMenuBar"), tmpl);
}

void MainMenuBarWidget::New(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args.GetIsolate());
  new MainMenuBarWidget(env->principal_realm(), args.This());
}

void MainMenuBarWidget::Render() {
  if (ImGui::BeginMainMenuBar()) {
    RenderChildren();
    ImGui::EndMainMenuBar();
  }
}

void MenuBarWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  tmpl->SetClassName(FixedOneByteString(isolate, "MenuBar"));
  target->Set(FixedOneByteString(isolate, "MenuBar"), tmpl);
}

void MenuBarWidget::New(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args.GetIsolate());
  new MenuBarWidget(env->principal_realm(), args.This());
}

void MenuBarWidget::Render() {
  if (ImGui::BeginMenuBar()) {
    RenderChildren();
    ImGui::EndMenuBar();
  }
}

MenuWidget::MenuWidget(Realm* realm, Local<Object> object, const std::string& label) : Widget(realm, object, label) {}

void MenuWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  tmpl->SetClassName(FixedOneByteString(isolate, "Menu"));
  target->Set(FixedOneByteString(isolate, "Menu"), tmpl);
}

void MenuWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Utf8Value label(isolate, args[0]);
  new MenuWidget(env->principal_realm(), args.This(), *label);
}

void MenuWidget::Render() {
  if (ImGui::BeginMenu(label_.c_str())) {
    RenderChildren();
    ImGui::EndMenu();
  }
}

MenuItemWidget::MenuItemWidget(
    Realm* realm, Local<Object> object, const std::string& label, const std::string& shortcut, bool selected)
    : Widget(realm, object, label), shortcut_(shortcut), selected_(selected) {}

void MenuItemWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  SetProtoProperty(isolate, tmpl, "clicked", GetClicked);
  SetProtoProperty(isolate, tmpl, "selected", GetSelected, SetSelected);

  tmpl->SetClassName(FixedOneByteString(isolate, "MenuItem"));
  target->Set(FixedOneByteString(isolate, "MenuItem"), tmpl);
}

void MenuItemWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Utf8Value label(isolate, args[0]);
  std::string shortcut;
  if (args.Length() > 1 && !args[1]->IsUndefined()) {
    Utf8Value sc(isolate, args[1]);
    shortcut = *sc;
  }
  bool selected = args.Length() > 2 ? args[2]->BooleanValue(isolate) : false;
  new MenuItemWidget(env->principal_realm(), args.This(), *label, shortcut, selected);
}

void MenuItemWidget::GetClicked(const FunctionCallbackInfo<Value>& args) {
  MenuItemWidget* self = BaseObject::Unwrap<MenuItemWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->clicked());
}

void MenuItemWidget::GetSelected(const FunctionCallbackInfo<Value>& args) {
  MenuItemWidget* self = BaseObject::Unwrap<MenuItemWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->selected());
}

void MenuItemWidget::SetSelected(const FunctionCallbackInfo<Value>& args) {
  MenuItemWidget* self = BaseObject::Unwrap<MenuItemWidget>(args.This());
  if (self) self->set_selected(args[0]->BooleanValue(args.GetIsolate()));
}

void MenuItemWidget::Render() {
  bool old_selected = selected_;
  clicked_ = ImGui::MenuItem(label_.c_str(), shortcut_.empty() ? nullptr : shortcut_.c_str(), &selected_);
  if (clicked_) {
    EmitEvent("click");
  }
  if (selected_ != old_selected) {
    Isolate* iso = isolate();
    HandleScope scope(iso);
    EmitEvent("change", Boolean::New(iso, selected_));
  }
}

}  // namespace nyx
