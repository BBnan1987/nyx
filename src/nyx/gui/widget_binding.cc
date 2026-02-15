#include "nyx/env.h"
#include "nyx/gui/colors.h"
#include "nyx/gui/combo.h"
#include "nyx/gui/common.h"
#include "nyx/gui/image.h"
#include "nyx/gui/input.h"
#include "nyx/gui/layout.h"
#include "nyx/gui/listbox.h"
#include "nyx/gui/menus.h"
#include "nyx/gui/panel.h"
#include "nyx/gui/plotting.h"
#include "nyx/gui/popups.h"
#include "nyx/gui/selectable.h"
#include "nyx/gui/slider.h"
#include "nyx/gui/tables.h"
#include "nyx/gui/tabs.h"
#include "nyx/gui/text.h"
#include "nyx/gui/tooltip.h"
#include "nyx/gui/trees.h"
#include "nyx/gui/widget.h"
#include "nyx/gui/widget_manager.h"
#include "nyx/nyx_binding.h"
#include "nyx/util.h"

#include <imgui.h>

namespace nyx {

using v8::Boolean;
using v8::Context;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Integer;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::ObjectTemplate;
using v8::String;
using v8::Value;

static void WidgetAdd(const FunctionCallbackInfo<Value>& args) {
  Widget* self;
  ASSIGN_OR_RETURN_UNWRAP(&self, args.This());
  Widget* child;
  ASSIGN_OR_RETURN_UNWRAP(&child, args[0]);
  self->AddChild(child);
  args.GetReturnValue().Set(args[0]);
}

static void WidgetRemove(const FunctionCallbackInfo<Value>& args) {
  Widget* self;
  ASSIGN_OR_RETURN_UNWRAP(&self, args.This());
  Widget* child;
  ASSIGN_OR_RETURN_UNWRAP(&child, args[0]);
  self->RemoveChild(child);
}

static void WidgetOn(const FunctionCallbackInfo<Value>& args) {
  Widget* self;
  ASSIGN_OR_RETURN_UNWRAP(&self, args.This());
  Isolate* isolate = args.GetIsolate();
  Utf8Value event(isolate, args[0]);
  if (args.Length() > 1 && args[1]->IsFunction()) {
    self->On(*event, args[1].As<Function>());
  }
}

static void WidgetOff(const FunctionCallbackInfo<Value>& args) {
  Widget* self;
  ASSIGN_OR_RETURN_UNWRAP(&self, args.This());
  Isolate* isolate = args.GetIsolate();
  Utf8Value event(isolate, args[0]);
  self->Off(*event);
}

static void WidgetDestroy(const FunctionCallbackInfo<Value>& args) {
  Widget* self;
  ASSIGN_OR_RETURN_UNWRAP(&self, args.This());

  // Remove from parent or widget manager
  if (self->parent()) {
    self->parent()->RemoveChild(self);
  } else {
    Environment* env = Environment::GetCurrent(args);
    if (env->widget_manager()) {
      env->widget_manager()->RemoveRoot(self);
    }
  }
  self->ClearChildren();
  self->MakeWeak();
}

static void WidgetGetVisible(const FunctionCallbackInfo<Value>& args) {
  Widget* self;
  ASSIGN_OR_RETURN_UNWRAP(&self, args.This());
  args.GetReturnValue().Set(self->visible());
}

static void WidgetSetVisible(const FunctionCallbackInfo<Value>& args) {
  Widget* self;
  ASSIGN_OR_RETURN_UNWRAP(&self, args.This());
  self->set_visible(args[0]->BooleanValue(args.GetIsolate()));
}

static void InstallWidgetMethods(Isolate* isolate, Local<ObjectTemplate> proto) {
  SetMethod(isolate, proto, "add", WidgetAdd);
  SetMethod(isolate, proto, "remove", WidgetRemove);
  SetMethod(isolate, proto, "on", WidgetOn);
  SetMethod(isolate, proto, "off", WidgetOff);
  SetMethod(isolate, proto, "destroy", WidgetDestroy);
  SetProperty(isolate, proto, "visible", WidgetGetVisible, WidgetSetVisible);
}

static Local<FunctionTemplate> NewWidgetTemplate(Isolate* isolate) {
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  InstallWidgetMethods(isolate, tmpl->PrototypeTemplate());
  return tmpl;
}

static void PanelNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);

  Utf8Value title(isolate, args[0]);
  ImGuiWindowFlags flags =
      args.Length() > 1 ? static_cast<ImGuiWindowFlags>(args[1]->Uint32Value(context).FromMaybe(0)) : 0;

  PanelWidget* panel = new PanelWidget(env->principal_realm(), args.This(), *title, flags);
  if (env->widget_manager()) {
    env->widget_manager()->AddRoot(panel);
  }
}

static void PanelGetOpen(const FunctionCallbackInfo<Value>& args) {
  PanelWidget* self = BaseObject::Unwrap<PanelWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->open());
}

static void PanelSetOpen(const FunctionCallbackInfo<Value>& args) {
  PanelWidget* self = BaseObject::Unwrap<PanelWidget>(args.This());
  if (self) self->set_open(args[0]->BooleanValue(args.GetIsolate()));
}

static void PanelGetTitle(const FunctionCallbackInfo<Value>& args) {
  PanelWidget* self = BaseObject::Unwrap<PanelWidget>(args.This());
  if (self) {
    Isolate* iso = args.GetIsolate();
    args.GetReturnValue().Set(String::NewFromUtf8(iso, self->title().c_str()).ToLocalChecked());
  }
}

static void PanelSetTitle(const FunctionCallbackInfo<Value>& args) {
  PanelWidget* self = BaseObject::Unwrap<PanelWidget>(args.This());
  if (self) {
    Utf8Value title(args.GetIsolate(), args[0]);
    self->set_title(*title);
  }
}

static void PanelGetFlags(const FunctionCallbackInfo<Value>& args) {
  PanelWidget* self = BaseObject::Unwrap<PanelWidget>(args.This());
  if (self) args.GetReturnValue().Set(static_cast<uint32_t>(self->flags()));
}

static void PanelSetFlags(const FunctionCallbackInfo<Value>& args) {
  PanelWidget* self = BaseObject::Unwrap<PanelWidget>(args.This());
  if (self) {
    Local<Context> ctx = args.GetIsolate()->GetCurrentContext();
    self->set_flags(static_cast<ImGuiWindowFlags>(args[0]->Uint32Value(ctx).FromMaybe(0)));
  }
}

static void TextNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Utf8Value text(isolate, args[0]);
  new TextWidget(env->principal_realm(), args.This(), *text);
}

static void TextGetText(const FunctionCallbackInfo<Value>& args) {
  TextWidget* self = BaseObject::Unwrap<TextWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->text().c_str()).ToLocalChecked());
  }
}

static void TextSetText(const FunctionCallbackInfo<Value>& args) {
  TextWidget* self = BaseObject::Unwrap<TextWidget>(args.This());
  if (self) {
    Utf8Value text(args.GetIsolate(), args[0]);
    self->set_text(*text);
  }
}

static void TextColoredNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value text(isolate, args[0]);
  float r = static_cast<float>(args[1]->NumberValue(context).FromMaybe(1.0));
  float g = static_cast<float>(args[2]->NumberValue(context).FromMaybe(1.0));
  float b = static_cast<float>(args[3]->NumberValue(context).FromMaybe(1.0));
  float a = static_cast<float>(args[4]->NumberValue(context).FromMaybe(1.0));
  new TextColoredWidget(env->principal_realm(), args.This(), *text, r, g, b, a);
}

static void ButtonNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value label(isolate, args[0]);
  float w = args.Length() > 1 ? static_cast<float>(args[1]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  float h = args.Length() > 2 ? static_cast<float>(args[2]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  new ButtonWidget(env->principal_realm(), args.This(), *label, w, h);
}

static void ButtonGetClicked(const FunctionCallbackInfo<Value>& args) {
  ButtonWidget* self = BaseObject::Unwrap<ButtonWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->clicked());
}

static void ButtonGetLabel(const FunctionCallbackInfo<Value>& args) {
  ButtonWidget* self = BaseObject::Unwrap<ButtonWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->label().c_str()).ToLocalChecked());
  }
}

static void ButtonSetLabel(const FunctionCallbackInfo<Value>& args) {
  ButtonWidget* self = BaseObject::Unwrap<ButtonWidget>(args.This());
  if (self) {
    Utf8Value label(args.GetIsolate(), args[0]);
    self->set_label(*label);
  }
}

static void InvisibleButtonNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value label(isolate, args[0]);
  float w = args.Length() > 1 ? static_cast<float>(args[1]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  float h = args.Length() > 2 ? static_cast<float>(args[2]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  new InvisibleButtonWidget(env->principal_realm(), args.This(), *label, w, h);
}

static void InvisibleButtonGetClicked(const FunctionCallbackInfo<Value>& args) {
  InvisibleButtonWidget* self = BaseObject::Unwrap<InvisibleButtonWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->clicked());
}

static void InvisibleButtonGetLabel(const FunctionCallbackInfo<Value>& args) {
  InvisibleButtonWidget* self = BaseObject::Unwrap<InvisibleButtonWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->label().c_str()).ToLocalChecked());
  }
}

static void InvisibleButtonSetLabel(const FunctionCallbackInfo<Value>& args) {
  InvisibleButtonWidget* self = BaseObject::Unwrap<InvisibleButtonWidget>(args.This());
  if (self) {
    Utf8Value label(args.GetIsolate(), args[0]);
    self->set_label(*label);
  }
}

static void CheckboxNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Utf8Value label(isolate, args[0]);
  bool checked = args.Length() > 1 ? args[1]->BooleanValue(isolate) : false;
  new CheckboxWidget(env->principal_realm(), args.This(), *label, checked);
}

static void CheckboxGetChecked(const FunctionCallbackInfo<Value>& args) {
  CheckboxWidget* self = BaseObject::Unwrap<CheckboxWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->checked());
}

static void CheckboxSetChecked(const FunctionCallbackInfo<Value>& args) {
  CheckboxWidget* self = BaseObject::Unwrap<CheckboxWidget>(args.This());
  if (self) self->set_checked(args[0]->BooleanValue(args.GetIsolate()));
}

static void CheckboxGetLabel(const FunctionCallbackInfo<Value>& args) {
  CheckboxWidget* self = BaseObject::Unwrap<CheckboxWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->label().c_str()).ToLocalChecked());
  }
}

static void CheckboxSetLabel(const FunctionCallbackInfo<Value>& args) {
  CheckboxWidget* self = BaseObject::Unwrap<CheckboxWidget>(args.This());
  if (self) {
    Utf8Value label(args.GetIsolate(), args[0]);
    self->set_label(*label);
  }
}

static void SliderFloatNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value label(isolate, args[0]);
  float min_val = static_cast<float>(args[1]->NumberValue(context).FromMaybe(0.0));
  float max_val = static_cast<float>(args[2]->NumberValue(context).FromMaybe(1.0));
  float value = args.Length() > 3 ? static_cast<float>(args[3]->NumberValue(context).FromMaybe(0.0)) : min_val;
  new SliderFloatWidget(env->principal_realm(), args.This(), *label, min_val, max_val, value);
}

static void SliderFloatGetValue(const FunctionCallbackInfo<Value>& args) {
  SliderFloatWidget* self = BaseObject::Unwrap<SliderFloatWidget>(args.This());
  if (self) args.GetReturnValue().Set(static_cast<double>(self->value()));
}

static void SliderFloatSetValue(const FunctionCallbackInfo<Value>& args) {
  SliderFloatWidget* self = BaseObject::Unwrap<SliderFloatWidget>(args.This());
  if (self) {
    Local<Context> ctx = args.GetIsolate()->GetCurrentContext();
    self->set_value(static_cast<float>(args[0]->NumberValue(ctx).FromMaybe(0.0)));
  }
}

static void SliderFloatGetLabel(const FunctionCallbackInfo<Value>& args) {
  SliderFloatWidget* self = BaseObject::Unwrap<SliderFloatWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->label().c_str()).ToLocalChecked());
  }
}

static void SliderFloatSetLabel(const FunctionCallbackInfo<Value>& args) {
  SliderFloatWidget* self = BaseObject::Unwrap<SliderFloatWidget>(args.This());
  if (self) {
    Utf8Value label(args.GetIsolate(), args[0]);
    self->set_label(*label);
  }
}

static void SliderIntNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value label(isolate, args[0]);
  int min_val = args[1]->Int32Value(context).FromMaybe(0);
  int max_val = args[2]->Int32Value(context).FromMaybe(100);
  int value = args.Length() > 3 ? args[3]->Int32Value(context).FromMaybe(min_val) : min_val;
  new SliderIntWidget(env->principal_realm(), args.This(), *label, min_val, max_val, value);
}

static void SliderIntGetValue(const FunctionCallbackInfo<Value>& args) {
  SliderIntWidget* self = BaseObject::Unwrap<SliderIntWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->value());
}

static void SliderIntSetValue(const FunctionCallbackInfo<Value>& args) {
  SliderIntWidget* self = BaseObject::Unwrap<SliderIntWidget>(args.This());
  if (self) {
    Local<Context> ctx = args.GetIsolate()->GetCurrentContext();
    self->set_value(args[0]->Int32Value(ctx).FromMaybe(0));
  }
}

static void SliderIntGetLabel(const FunctionCallbackInfo<Value>& args) {
  SliderIntWidget* self = BaseObject::Unwrap<SliderIntWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->label().c_str()).ToLocalChecked());
  }
}

static void SliderIntSetLabel(const FunctionCallbackInfo<Value>& args) {
  SliderIntWidget* self = BaseObject::Unwrap<SliderIntWidget>(args.This());
  if (self) {
    Utf8Value label(args.GetIsolate(), args[0]);
    self->set_label(*label);
  }
}

static void InputTextNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value label(isolate, args[0]);
  size_t max_length = args.Length() > 1 ? static_cast<size_t>(args[1]->Uint32Value(context).FromMaybe(256)) : 256;
  new InputTextWidget(env->principal_realm(), args.This(), *label, max_length);
}

static void InputTextGetText(const FunctionCallbackInfo<Value>& args) {
  InputTextWidget* self = BaseObject::Unwrap<InputTextWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->text().c_str()).ToLocalChecked());
  }
}

static void InputTextSetText(const FunctionCallbackInfo<Value>& args) {
  InputTextWidget* self = BaseObject::Unwrap<InputTextWidget>(args.This());
  if (self) {
    Utf8Value text(args.GetIsolate(), args[0]);
    self->set_text(*text);
  }
}

static void InputTextGetLabel(const FunctionCallbackInfo<Value>& args) {
  InputTextWidget* self = BaseObject::Unwrap<InputTextWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->label().c_str()).ToLocalChecked());
  }
}

static void InputTextSetLabel(const FunctionCallbackInfo<Value>& args) {
  InputTextWidget* self = BaseObject::Unwrap<InputTextWidget>(args.This());
  if (self) {
    Utf8Value label(args.GetIsolate(), args[0]);
    self->set_label(*label);
  }
}

static void SeparatorNew(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args.GetIsolate());
  new SeparatorWidget(env->principal_realm(), args.This());
}

static void SpacingNew(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args.GetIsolate());
  new SpacingWidget(env->principal_realm(), args.This());
}

static void SameLineNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  float offset = args.Length() > 0 ? static_cast<float>(args[0]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  float spacing = args.Length() > 1 ? static_cast<float>(args[1]->NumberValue(context).FromMaybe(-1.0)) : -1.0f;
  new SameLineWidget(env->principal_realm(), args.This(), offset, spacing);
}

static void TreeNodeNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Utf8Value label(isolate, args[0]);
  new TreeNodeWidget(env->principal_realm(), args.This(), *label);
}

static void TreeNodeGetOpen(const FunctionCallbackInfo<Value>& args) {
  TreeNodeWidget* self = BaseObject::Unwrap<TreeNodeWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->open());
}

static void TreeNodeGetLabel(const FunctionCallbackInfo<Value>& args) {
  TreeNodeWidget* self = BaseObject::Unwrap<TreeNodeWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->label().c_str()).ToLocalChecked());
  }
}

static void TreeNodeSetLabel(const FunctionCallbackInfo<Value>& args) {
  TreeNodeWidget* self = BaseObject::Unwrap<TreeNodeWidget>(args.This());
  if (self) {
    Utf8Value label(args.GetIsolate(), args[0]);
    self->set_label(*label);
  }
}

static void CollapsingHeaderNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value label(isolate, args[0]);
  ImGuiTreeNodeFlags flags =
      args.Length() > 1 ? static_cast<ImGuiTreeNodeFlags>(args[1]->Uint32Value(context).FromMaybe(0)) : 0;
  new CollapsingHeaderWidget(env->principal_realm(), args.This(), *label, flags);
}

static void CollapsingHeaderGetOpen(const FunctionCallbackInfo<Value>& args) {
  CollapsingHeaderWidget* self = BaseObject::Unwrap<CollapsingHeaderWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->open());
}

static void CollapsingHeaderGetLabel(const FunctionCallbackInfo<Value>& args) {
  CollapsingHeaderWidget* self = BaseObject::Unwrap<CollapsingHeaderWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->label().c_str()).ToLocalChecked());
  }
}

static void CollapsingHeaderSetLabel(const FunctionCallbackInfo<Value>& args) {
  CollapsingHeaderWidget* self = BaseObject::Unwrap<CollapsingHeaderWidget>(args.This());
  if (self) {
    Utf8Value label(args.GetIsolate(), args[0]);
    self->set_label(*label);
  }
}

static void TabBarNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Utf8Value id(isolate, args[0]);
  new TabBarWidget(env->principal_realm(), args.This(), *id);
}

static void TabItemNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Utf8Value label(isolate, args[0]);
  new TabItemWidget(env->principal_realm(), args.This(), *label);
}

static void TabItemGetSelected(const FunctionCallbackInfo<Value>& args) {
  TabItemWidget* self = BaseObject::Unwrap<TabItemWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->selected());
}

static void TabItemGetLabel(const FunctionCallbackInfo<Value>& args) {
  TabItemWidget* self = BaseObject::Unwrap<TabItemWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->label().c_str()).ToLocalChecked());
  }
}

static void TabItemSetLabel(const FunctionCallbackInfo<Value>& args) {
  TabItemWidget* self = BaseObject::Unwrap<TabItemWidget>(args.This());
  if (self) {
    Utf8Value label(args.GetIsolate(), args[0]);
    self->set_label(*label);
  }
}

static void DemoWindowNew(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args.GetIsolate());
  DemoWindowWidget* demo = new DemoWindowWidget(env->principal_realm(), args.This());
  if (env->widget_manager()) {
    env->widget_manager()->AddRoot(demo);
  }
}

static void BulletTextNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Utf8Value text(isolate, args[0]);
  new BulletTextWidget(env->principal_realm(), args.This(), *text);
}

static void TextWrappedNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Utf8Value text(isolate, args[0]);
  new TextWrappedWidget(env->principal_realm(), args.This(), *text);
}

static void TextDisabledNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Utf8Value text(isolate, args[0]);
  new TextDisabledWidget(env->principal_realm(), args.This(), *text);
}

static void LabelTextNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Utf8Value label(isolate, args[0]);
  Utf8Value text(isolate, args[1]);
  new LabelTextWidget(env->principal_realm(), args.This(), *label, *text);
}

static void LabelTextGetLabel(const FunctionCallbackInfo<Value>& args) {
  LabelTextWidget* self = BaseObject::Unwrap<LabelTextWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->label().c_str()).ToLocalChecked());
  }
}

static void LabelTextSetLabel(const FunctionCallbackInfo<Value>& args) {
  LabelTextWidget* self = BaseObject::Unwrap<LabelTextWidget>(args.This());
  if (self) {
    Utf8Value label(args.GetIsolate(), args[0]);
    self->set_label(*label);
  }
}

static void SeparatorTextNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Utf8Value text(isolate, args[0]);
  new SeparatorTextWidget(env->principal_realm(), args.This(), *text);
}

static void BulletNew(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args.GetIsolate());
  new BulletWidget(env->principal_realm(), args.This());
}

static void NewLineNew(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args.GetIsolate());
  new NewLineWidget(env->principal_realm(), args.This());
}

static void SmallButtonNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Utf8Value label(isolate, args[0]);
  new SmallButtonWidget(env->principal_realm(), args.This(), *label);
}

static void SmallButtonGetClicked(const FunctionCallbackInfo<Value>& args) {
  SmallButtonWidget* self = BaseObject::Unwrap<SmallButtonWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->clicked());
}

static void SmallButtonGetLabel(const FunctionCallbackInfo<Value>& args) {
  SmallButtonWidget* self = BaseObject::Unwrap<SmallButtonWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->label().c_str()).ToLocalChecked());
  }
}

static void SmallButtonSetLabel(const FunctionCallbackInfo<Value>& args) {
  SmallButtonWidget* self = BaseObject::Unwrap<SmallButtonWidget>(args.This());
  if (self) {
    Utf8Value label(args.GetIsolate(), args[0]);
    self->set_label(*label);
  }
}

static void ArrowButtonNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value id(isolate, args[0]);
  ImGuiDir dir = static_cast<ImGuiDir>(args[1]->Int32Value(context).FromMaybe(0));
  new ArrowButtonWidget(env->principal_realm(), args.This(), *id, dir);
}

static void ArrowButtonGetClicked(const FunctionCallbackInfo<Value>& args) {
  ArrowButtonWidget* self = BaseObject::Unwrap<ArrowButtonWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->clicked());
}

static void RadioButtonNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Utf8Value label(isolate, args[0]);
  bool active = args.Length() > 1 ? args[1]->BooleanValue(isolate) : false;
  new RadioButtonWidget(env->principal_realm(), args.This(), *label, active);
}

static void RadioButtonGetActive(const FunctionCallbackInfo<Value>& args) {
  RadioButtonWidget* self = BaseObject::Unwrap<RadioButtonWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->active());
}

static void RadioButtonSetActive(const FunctionCallbackInfo<Value>& args) {
  RadioButtonWidget* self = BaseObject::Unwrap<RadioButtonWidget>(args.This());
  if (self) self->set_active(args[0]->BooleanValue(args.GetIsolate()));
}

static void RadioButtonGetClicked(const FunctionCallbackInfo<Value>& args) {
  RadioButtonWidget* self = BaseObject::Unwrap<RadioButtonWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->clicked());
}

static void RadioButtonGetLabel(const FunctionCallbackInfo<Value>& args) {
  RadioButtonWidget* self = BaseObject::Unwrap<RadioButtonWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->label().c_str()).ToLocalChecked());
  }
}

static void RadioButtonSetLabel(const FunctionCallbackInfo<Value>& args) {
  RadioButtonWidget* self = BaseObject::Unwrap<RadioButtonWidget>(args.This());
  if (self) {
    Utf8Value label(args.GetIsolate(), args[0]);
    self->set_label(*label);
  }
}

static void DragFloatNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value label(isolate, args[0]);
  float value = static_cast<float>(args[1]->NumberValue(context).FromMaybe(0.0));
  float speed = args.Length() > 2 ? static_cast<float>(args[2]->NumberValue(context).FromMaybe(1.0)) : 1.0f;
  float min_val = args.Length() > 3 ? static_cast<float>(args[3]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  float max_val = args.Length() > 4 ? static_cast<float>(args[4]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  new DragFloatWidget(env->principal_realm(), args.This(), *label, value, speed, min_val, max_val);
}

static void DragFloatGetValue(const FunctionCallbackInfo<Value>& args) {
  DragFloatWidget* self = BaseObject::Unwrap<DragFloatWidget>(args.This());
  if (self) args.GetReturnValue().Set(static_cast<double>(self->value()));
}

static void DragFloatSetValue(const FunctionCallbackInfo<Value>& args) {
  DragFloatWidget* self = BaseObject::Unwrap<DragFloatWidget>(args.This());
  if (self) {
    Local<Context> ctx = args.GetIsolate()->GetCurrentContext();
    self->set_value(static_cast<float>(args[0]->NumberValue(ctx).FromMaybe(0.0)));
  }
}

static void DragFloatGetLabel(const FunctionCallbackInfo<Value>& args) {
  DragFloatWidget* self = BaseObject::Unwrap<DragFloatWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->label().c_str()).ToLocalChecked());
  }
}

static void DragFloatSetLabel(const FunctionCallbackInfo<Value>& args) {
  DragFloatWidget* self = BaseObject::Unwrap<DragFloatWidget>(args.This());
  if (self) {
    Utf8Value label(args.GetIsolate(), args[0]);
    self->set_label(*label);
  }
}

static void DragIntNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value label(isolate, args[0]);
  int value = args[1]->Int32Value(context).FromMaybe(0);
  float speed = args.Length() > 2 ? static_cast<float>(args[2]->NumberValue(context).FromMaybe(1.0)) : 1.0f;
  int min_val = args.Length() > 3 ? args[3]->Int32Value(context).FromMaybe(0) : 0;
  int max_val = args.Length() > 4 ? args[4]->Int32Value(context).FromMaybe(0) : 0;
  new DragIntWidget(env->principal_realm(), args.This(), *label, value, speed, min_val, max_val);
}

static void DragIntGetValue(const FunctionCallbackInfo<Value>& args) {
  DragIntWidget* self = BaseObject::Unwrap<DragIntWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->value());
}

static void DragIntSetValue(const FunctionCallbackInfo<Value>& args) {
  DragIntWidget* self = BaseObject::Unwrap<DragIntWidget>(args.This());
  if (self) {
    Local<Context> ctx = args.GetIsolate()->GetCurrentContext();
    self->set_value(args[0]->Int32Value(ctx).FromMaybe(0));
  }
}

static void DragIntGetLabel(const FunctionCallbackInfo<Value>& args) {
  DragIntWidget* self = BaseObject::Unwrap<DragIntWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->label().c_str()).ToLocalChecked());
  }
}

static void DragIntSetLabel(const FunctionCallbackInfo<Value>& args) {
  DragIntWidget* self = BaseObject::Unwrap<DragIntWidget>(args.This());
  if (self) {
    Utf8Value label(args.GetIsolate(), args[0]);
    self->set_label(*label);
  }
}

static void InputFloatNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value label(isolate, args[0]);
  float value = args.Length() > 1 ? static_cast<float>(args[1]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  new InputFloatWidget(env->principal_realm(), args.This(), *label, value);
}

static void InputFloatGetValue(const FunctionCallbackInfo<Value>& args) {
  InputFloatWidget* self = BaseObject::Unwrap<InputFloatWidget>(args.This());
  if (self) args.GetReturnValue().Set(static_cast<double>(self->value()));
}

static void InputFloatSetValue(const FunctionCallbackInfo<Value>& args) {
  InputFloatWidget* self = BaseObject::Unwrap<InputFloatWidget>(args.This());
  if (self) {
    Local<Context> ctx = args.GetIsolate()->GetCurrentContext();
    self->set_value(static_cast<float>(args[0]->NumberValue(ctx).FromMaybe(0.0)));
  }
}

static void InputFloatGetLabel(const FunctionCallbackInfo<Value>& args) {
  InputFloatWidget* self = BaseObject::Unwrap<InputFloatWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->label().c_str()).ToLocalChecked());
  }
}

static void InputFloatSetLabel(const FunctionCallbackInfo<Value>& args) {
  InputFloatWidget* self = BaseObject::Unwrap<InputFloatWidget>(args.This());
  if (self) {
    Utf8Value label(args.GetIsolate(), args[0]);
    self->set_label(*label);
  }
}

static void InputIntNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value label(isolate, args[0]);
  int value = args.Length() > 1 ? args[1]->Int32Value(context).FromMaybe(0) : 0;
  new InputIntWidget(env->principal_realm(), args.This(), *label, value);
}

static void InputIntGetValue(const FunctionCallbackInfo<Value>& args) {
  InputIntWidget* self = BaseObject::Unwrap<InputIntWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->value());
}

static void InputIntSetValue(const FunctionCallbackInfo<Value>& args) {
  InputIntWidget* self = BaseObject::Unwrap<InputIntWidget>(args.This());
  if (self) {
    Local<Context> ctx = args.GetIsolate()->GetCurrentContext();
    self->set_value(args[0]->Int32Value(ctx).FromMaybe(0));
  }
}

static void InputIntGetLabel(const FunctionCallbackInfo<Value>& args) {
  InputIntWidget* self = BaseObject::Unwrap<InputIntWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->label().c_str()).ToLocalChecked());
  }
}

static void InputIntSetLabel(const FunctionCallbackInfo<Value>& args) {
  InputIntWidget* self = BaseObject::Unwrap<InputIntWidget>(args.This());
  if (self) {
    Utf8Value label(args.GetIsolate(), args[0]);
    self->set_label(*label);
  }
}

static void InputTextMultilineNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value label(isolate, args[0]);
  size_t max_length = args.Length() > 1 ? static_cast<size_t>(args[1]->Uint32Value(context).FromMaybe(1024)) : 1024;
  float width = args.Length() > 2 ? static_cast<float>(args[2]->NumberValue(context).FromMaybe(-1.0)) : -1.0f;
  float height = args.Length() > 3 ? static_cast<float>(args[3]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  new InputTextMultilineWidget(env->principal_realm(), args.This(), *label, max_length, width, height);
}

static void InputTextMultilineGetText(const FunctionCallbackInfo<Value>& args) {
  InputTextMultilineWidget* self = BaseObject::Unwrap<InputTextMultilineWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->text().c_str()).ToLocalChecked());
  }
}

static void InputTextMultilineSetText(const FunctionCallbackInfo<Value>& args) {
  InputTextMultilineWidget* self = BaseObject::Unwrap<InputTextMultilineWidget>(args.This());
  if (self) {
    Utf8Value text(args.GetIsolate(), args[0]);
    self->set_text(*text);
  }
}

static void InputTextMultilineGetLabel(const FunctionCallbackInfo<Value>& args) {
  InputTextMultilineWidget* self = BaseObject::Unwrap<InputTextMultilineWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->label().c_str()).ToLocalChecked());
  }
}

static void InputTextMultilineSetLabel(const FunctionCallbackInfo<Value>& args) {
  InputTextMultilineWidget* self = BaseObject::Unwrap<InputTextMultilineWidget>(args.This());
  if (self) {
    Utf8Value label(args.GetIsolate(), args[0]);
    self->set_label(*label);
  }
}

static void InputTextWithHintNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value label(isolate, args[0]);
  size_t max_length = args.Length() > 1 ? static_cast<size_t>(args[1]->Uint32Value(context).FromMaybe(256)) : 256;
  new InputTextWithHintWidget(env->principal_realm(), args.This(), *label, max_length);
}

static void InputTextWithHintGetText(const FunctionCallbackInfo<Value>& args) {
  InputTextWithHintWidget* self = BaseObject::Unwrap<InputTextWithHintWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->text().c_str()).ToLocalChecked());
  }
}

static void InputTextWithHintSetText(const FunctionCallbackInfo<Value>& args) {
  InputTextWithHintWidget* self = BaseObject::Unwrap<InputTextWithHintWidget>(args.This());
  if (self) {
    Utf8Value text(args.GetIsolate(), args[0]);
    self->set_text(*text);
  }
}

static void InputTextWithHintGetHint(const FunctionCallbackInfo<Value>& args) {
  InputTextWithHintWidget* self = BaseObject::Unwrap<InputTextWithHintWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->hint().c_str()).ToLocalChecked());
  }
}

static void InputTextWithHintSetHint(const FunctionCallbackInfo<Value>& args) {
  InputTextWithHintWidget* self = BaseObject::Unwrap<InputTextWithHintWidget>(args.This());
  if (self) {
    Utf8Value hint(args.GetIsolate(), args[0]);
    self->set_hint(*hint);
  }
}

static void InputTextWithHintGetLabel(const FunctionCallbackInfo<Value>& args) {
  InputTextWithHintWidget* self = BaseObject::Unwrap<InputTextWithHintWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->label().c_str()).ToLocalChecked());
  }
}

static void InputTextWithHintSetLabel(const FunctionCallbackInfo<Value>& args) {
  InputTextWithHintWidget* self = BaseObject::Unwrap<InputTextWithHintWidget>(args.This());
  if (self) {
    Utf8Value label(args.GetIsolate(), args[0]);
    self->set_label(*label);
  }
}

static void ColorEdit3New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value label(isolate, args[0]);
  float r = args.Length() > 1 ? static_cast<float>(args[1]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  float g = args.Length() > 2 ? static_cast<float>(args[2]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  float b = args.Length() > 3 ? static_cast<float>(args[3]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  new ColorEdit3Widget(env->principal_realm(), args.This(), *label, r, g, b);
}

static void ColorEdit3GetR(const FunctionCallbackInfo<Value>& args) {
  ColorEdit3Widget* self = BaseObject::Unwrap<ColorEdit3Widget>(args.This());
  if (self) args.GetReturnValue().Set(static_cast<double>(self->r()));
}

static void ColorEdit3GetG(const FunctionCallbackInfo<Value>& args) {
  ColorEdit3Widget* self = BaseObject::Unwrap<ColorEdit3Widget>(args.This());
  if (self) args.GetReturnValue().Set(static_cast<double>(self->g()));
}

static void ColorEdit3GetB(const FunctionCallbackInfo<Value>& args) {
  ColorEdit3Widget* self = BaseObject::Unwrap<ColorEdit3Widget>(args.This());
  if (self) args.GetReturnValue().Set(static_cast<double>(self->b()));
}

static void ColorEdit3GetLabel(const FunctionCallbackInfo<Value>& args) {
  ColorEdit3Widget* self = BaseObject::Unwrap<ColorEdit3Widget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->label().c_str()).ToLocalChecked());
  }
}

static void ColorEdit4New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value label(isolate, args[0]);
  float r = args.Length() > 1 ? static_cast<float>(args[1]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  float g = args.Length() > 2 ? static_cast<float>(args[2]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  float b = args.Length() > 3 ? static_cast<float>(args[3]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  float a = args.Length() > 4 ? static_cast<float>(args[4]->NumberValue(context).FromMaybe(1.0)) : 1.0f;
  new ColorEdit4Widget(env->principal_realm(), args.This(), *label, r, g, b, a);
}

static void ColorEdit4GetR(const FunctionCallbackInfo<Value>& args) {
  ColorEdit4Widget* self = BaseObject::Unwrap<ColorEdit4Widget>(args.This());
  if (self) args.GetReturnValue().Set(static_cast<double>(self->r()));
}

static void ColorEdit4GetG(const FunctionCallbackInfo<Value>& args) {
  ColorEdit4Widget* self = BaseObject::Unwrap<ColorEdit4Widget>(args.This());
  if (self) args.GetReturnValue().Set(static_cast<double>(self->g()));
}

static void ColorEdit4GetB(const FunctionCallbackInfo<Value>& args) {
  ColorEdit4Widget* self = BaseObject::Unwrap<ColorEdit4Widget>(args.This());
  if (self) args.GetReturnValue().Set(static_cast<double>(self->b()));
}

static void ColorEdit4GetA(const FunctionCallbackInfo<Value>& args) {
  ColorEdit4Widget* self = BaseObject::Unwrap<ColorEdit4Widget>(args.This());
  if (self) args.GetReturnValue().Set(static_cast<double>(self->a()));
}

static void ColorEdit4GetLabel(const FunctionCallbackInfo<Value>& args) {
  ColorEdit4Widget* self = BaseObject::Unwrap<ColorEdit4Widget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->label().c_str()).ToLocalChecked());
  }
}

static void ColorPicker3New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value label(isolate, args[0]);
  float r = args.Length() > 1 ? static_cast<float>(args[1]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  float g = args.Length() > 2 ? static_cast<float>(args[2]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  float b = args.Length() > 3 ? static_cast<float>(args[3]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  new ColorPicker3Widget(env->principal_realm(), args.This(), *label, r, g, b);
}

static void ColorPicker3GetR(const FunctionCallbackInfo<Value>& args) {
  ColorPicker3Widget* self = BaseObject::Unwrap<ColorPicker3Widget>(args.This());
  if (self) args.GetReturnValue().Set(static_cast<double>(self->r()));
}

static void ColorPicker3GetG(const FunctionCallbackInfo<Value>& args) {
  ColorPicker3Widget* self = BaseObject::Unwrap<ColorPicker3Widget>(args.This());
  if (self) args.GetReturnValue().Set(static_cast<double>(self->g()));
}

static void ColorPicker3GetB(const FunctionCallbackInfo<Value>& args) {
  ColorPicker3Widget* self = BaseObject::Unwrap<ColorPicker3Widget>(args.This());
  if (self) args.GetReturnValue().Set(static_cast<double>(self->b()));
}

static void ColorPicker3GetLabel(const FunctionCallbackInfo<Value>& args) {
  ColorPicker3Widget* self = BaseObject::Unwrap<ColorPicker3Widget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->label().c_str()).ToLocalChecked());
  }
}

static void ColorPicker4New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value label(isolate, args[0]);
  float r = args.Length() > 1 ? static_cast<float>(args[1]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  float g = args.Length() > 2 ? static_cast<float>(args[2]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  float b = args.Length() > 3 ? static_cast<float>(args[3]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  float a = args.Length() > 4 ? static_cast<float>(args[4]->NumberValue(context).FromMaybe(1.0)) : 1.0f;
  float ref_r = args.Length() > 1 ? static_cast<float>(args[5]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  float ref_g = args.Length() > 2 ? static_cast<float>(args[6]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  float ref_b = args.Length() > 3 ? static_cast<float>(args[7]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  float ref_a = args.Length() > 4 ? static_cast<float>(args[8]->NumberValue(context).FromMaybe(1.0)) : 1.0f;
  new ColorPicker4Widget(env->principal_realm(), args.This(), *label, r, g, b, a, ref_r, ref_g, ref_b, ref_a);
}

static void ColorPicker4GetR(const FunctionCallbackInfo<Value>& args) {
  ColorPicker4Widget* self = BaseObject::Unwrap<ColorPicker4Widget>(args.This());
  if (self) args.GetReturnValue().Set(static_cast<double>(self->r()));
}

static void ColorPicker4GetG(const FunctionCallbackInfo<Value>& args) {
  ColorPicker4Widget* self = BaseObject::Unwrap<ColorPicker4Widget>(args.This());
  if (self) args.GetReturnValue().Set(static_cast<double>(self->g()));
}

static void ColorPicker4GetB(const FunctionCallbackInfo<Value>& args) {
  ColorPicker4Widget* self = BaseObject::Unwrap<ColorPicker4Widget>(args.This());
  if (self) args.GetReturnValue().Set(static_cast<double>(self->b()));
}

static void ColorPicker4GetA(const FunctionCallbackInfo<Value>& args) {
  ColorPicker4Widget* self = BaseObject::Unwrap<ColorPicker4Widget>(args.This());
  if (self) args.GetReturnValue().Set(static_cast<double>(self->a()));
}

static void ColorPicker4GetRefR(const FunctionCallbackInfo<Value>& args) {
  ColorPicker4Widget* self = BaseObject::Unwrap<ColorPicker4Widget>(args.This());
  if (self) args.GetReturnValue().Set(static_cast<double>(self->ref_r()));
}

static void ColorPicker4GetRefG(const FunctionCallbackInfo<Value>& args) {
  ColorPicker4Widget* self = BaseObject::Unwrap<ColorPicker4Widget>(args.This());
  if (self) args.GetReturnValue().Set(static_cast<double>(self->ref_g()));
}

static void ColorPicker4GetRefB(const FunctionCallbackInfo<Value>& args) {
  ColorPicker4Widget* self = BaseObject::Unwrap<ColorPicker4Widget>(args.This());
  if (self) args.GetReturnValue().Set(static_cast<double>(self->ref_b()));
}

static void ColorPicker4GetRefA(const FunctionCallbackInfo<Value>& args) {
  ColorPicker4Widget* self = BaseObject::Unwrap<ColorPicker4Widget>(args.This());
  if (self) args.GetReturnValue().Set(static_cast<double>(self->ref_a()));
}

static void ColorPicker4GetLabel(const FunctionCallbackInfo<Value>& args) {
  ColorPicker4Widget* self = BaseObject::Unwrap<ColorPicker4Widget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->label().c_str()).ToLocalChecked());
  }
}

static void ColorButtonNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value label(isolate, args[0]);
  float r = args.Length() > 1 ? static_cast<float>(args[1]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  float g = args.Length() > 2 ? static_cast<float>(args[2]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  float b = args.Length() > 3 ? static_cast<float>(args[3]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  float a = args.Length() > 4 ? static_cast<float>(args[4]->NumberValue(context).FromMaybe(1.0)) : 1.0f;
  float size_x = args.Length() > 1 ? static_cast<float>(args[5]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  float size_y = args.Length() > 2 ? static_cast<float>(args[6]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  new ColorButtonWidget(env->principal_realm(), args.This(), *label, r, g, b, a, size_x, size_y);
}

static void ColorButtonGetR(const FunctionCallbackInfo<Value>& args) {
  ColorButtonWidget* self = BaseObject::Unwrap<ColorButtonWidget>(args.This());
  if (self) args.GetReturnValue().Set(static_cast<double>(self->r()));
}

static void ColorButtonGetG(const FunctionCallbackInfo<Value>& args) {
  ColorButtonWidget* self = BaseObject::Unwrap<ColorButtonWidget>(args.This());
  if (self) args.GetReturnValue().Set(static_cast<double>(self->g()));
}

static void ColorButtonGetB(const FunctionCallbackInfo<Value>& args) {
  ColorButtonWidget* self = BaseObject::Unwrap<ColorButtonWidget>(args.This());
  if (self) args.GetReturnValue().Set(static_cast<double>(self->b()));
}

static void ColorButtonGetA(const FunctionCallbackInfo<Value>& args) {
  ColorButtonWidget* self = BaseObject::Unwrap<ColorButtonWidget>(args.This());
  if (self) args.GetReturnValue().Set(static_cast<double>(self->a()));
}

static void ColorButtonGetSizeX(const FunctionCallbackInfo<Value>& args) {
  ColorButtonWidget* self = BaseObject::Unwrap<ColorButtonWidget>(args.This());
  if (self) args.GetReturnValue().Set(static_cast<double>(self->size_x()));
}

static void ColorButtonGetSizeY(const FunctionCallbackInfo<Value>& args) {
  ColorButtonWidget* self = BaseObject::Unwrap<ColorButtonWidget>(args.This());
  if (self) args.GetReturnValue().Set(static_cast<double>(self->size_y()));
}

static void ColorButtonGetClicked(const FunctionCallbackInfo<Value>& args) {
  ColorButtonWidget* self = BaseObject::Unwrap<ColorButtonWidget>(args.This());
  if (self) args.GetReturnValue().Set(static_cast<double>(self->clicked()));
}

static void ColorButtonGetLabel(const FunctionCallbackInfo<Value>& args) {
  ColorButtonWidget* self = BaseObject::Unwrap<ColorButtonWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->label().c_str()).ToLocalChecked());
  }
}

static std::vector<std::string> ArrayToStringVector(Isolate* isolate, Local<Context> context, Local<Value> val) {
  std::vector<std::string> result;
  if (!val->IsArray()) return result;
  Local<v8::Array> arr = val.As<v8::Array>();
  for (uint32_t i = 0; i < arr->Length(); i++) {
    Local<Value> elem;
    if (arr->Get(context, i).ToLocal(&elem)) {
      Utf8Value str(isolate, elem);
      result.push_back(*str);
    }
  }
  return result;
}

static void ComboNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value label(isolate, args[0]);
  auto items = ArrayToStringVector(isolate, context, args[1]);
  int selected = args.Length() > 2 ? args[2]->Int32Value(context).FromMaybe(0) : 0;
  new ComboWidget(env->principal_realm(), args.This(), *label, std::move(items), selected);
}

static void ComboGetSelected(const FunctionCallbackInfo<Value>& args) {
  ComboWidget* self = BaseObject::Unwrap<ComboWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->selected());
}

static void ComboSetSelected(const FunctionCallbackInfo<Value>& args) {
  ComboWidget* self = BaseObject::Unwrap<ComboWidget>(args.This());
  if (self) {
    Local<Context> ctx = args.GetIsolate()->GetCurrentContext();
    self->set_selected(args[0]->Int32Value(ctx).FromMaybe(0));
  }
}

static void ComboGetLabel(const FunctionCallbackInfo<Value>& args) {
  ComboWidget* self = BaseObject::Unwrap<ComboWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->label().c_str()).ToLocalChecked());
  }
}

static void ComboSetItems(const FunctionCallbackInfo<Value>& args) {
  ComboWidget* self = BaseObject::Unwrap<ComboWidget>(args.This());
  if (self) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();
    self->set_items(ArrayToStringVector(isolate, context, args[0]));
  }
}

static void ListBoxNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value label(isolate, args[0]);
  auto items = ArrayToStringVector(isolate, context, args[1]);
  int selected = args.Length() > 2 ? args[2]->Int32Value(context).FromMaybe(0) : 0;
  int height = args.Length() > 3 ? args[3]->Int32Value(context).FromMaybe(-1) : -1;
  new ListBoxWidget(env->principal_realm(), args.This(), *label, std::move(items), selected, height);
}

static void ListBoxGetSelected(const FunctionCallbackInfo<Value>& args) {
  ListBoxWidget* self = BaseObject::Unwrap<ListBoxWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->selected());
}

static void ListBoxSetSelected(const FunctionCallbackInfo<Value>& args) {
  ListBoxWidget* self = BaseObject::Unwrap<ListBoxWidget>(args.This());
  if (self) {
    Local<Context> ctx = args.GetIsolate()->GetCurrentContext();
    self->set_selected(args[0]->Int32Value(ctx).FromMaybe(0));
  }
}

static void ListBoxGetLabel(const FunctionCallbackInfo<Value>& args) {
  ListBoxWidget* self = BaseObject::Unwrap<ListBoxWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->label().c_str()).ToLocalChecked());
  }
}

static void ListBoxSetItems(const FunctionCallbackInfo<Value>& args) {
  ListBoxWidget* self = BaseObject::Unwrap<ListBoxWidget>(args.This());
  if (self) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();
    self->set_items(ArrayToStringVector(isolate, context, args[0]));
  }
}

static void ProgressBarNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  float fraction = args.Length() > 0 ? static_cast<float>(args[0]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  std::string overlay;
  if (args.Length() > 1 && !args[1]->IsUndefined()) {
    Utf8Value ov(isolate, args[1]);
    overlay = *ov;
  }
  new ProgressBarWidget(env->principal_realm(), args.This(), fraction, overlay);
}

static void ProgressBarGetFraction(const FunctionCallbackInfo<Value>& args) {
  ProgressBarWidget* self = BaseObject::Unwrap<ProgressBarWidget>(args.This());
  if (self) args.GetReturnValue().Set(static_cast<double>(self->fraction()));
}

static void ProgressBarSetFraction(const FunctionCallbackInfo<Value>& args) {
  ProgressBarWidget* self = BaseObject::Unwrap<ProgressBarWidget>(args.This());
  if (self) {
    Local<Context> ctx = args.GetIsolate()->GetCurrentContext();
    self->set_fraction(static_cast<float>(args[0]->NumberValue(ctx).FromMaybe(0.0)));
  }
}

static void ProgressBarGetOverlay(const FunctionCallbackInfo<Value>& args) {
  ProgressBarWidget* self = BaseObject::Unwrap<ProgressBarWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->overlay().c_str()).ToLocalChecked());
  }
}

static void ProgressBarSetOverlay(const FunctionCallbackInfo<Value>& args) {
  ProgressBarWidget* self = BaseObject::Unwrap<ProgressBarWidget>(args.This());
  if (self) {
    Utf8Value ov(args.GetIsolate(), args[0]);
    self->set_overlay(*ov);
  }
}

static void SelectableNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Utf8Value label(isolate, args[0]);
  bool selected = args.Length() > 1 ? args[1]->BooleanValue(isolate) : false;
  new SelectableWidget(env->principal_realm(), args.This(), *label, selected);
}

static void SelectableGetSelected(const FunctionCallbackInfo<Value>& args) {
  SelectableWidget* self = BaseObject::Unwrap<SelectableWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->selected());
}

static void SelectableSetSelected(const FunctionCallbackInfo<Value>& args) {
  SelectableWidget* self = BaseObject::Unwrap<SelectableWidget>(args.This());
  if (self) self->set_selected(args[0]->BooleanValue(args.GetIsolate()));
}

static void SelectableGetLabel(const FunctionCallbackInfo<Value>& args) {
  SelectableWidget* self = BaseObject::Unwrap<SelectableWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->label().c_str()).ToLocalChecked());
  }
}

static void SelectableSetLabel(const FunctionCallbackInfo<Value>& args) {
  SelectableWidget* self = BaseObject::Unwrap<SelectableWidget>(args.This());
  if (self) {
    Utf8Value label(args.GetIsolate(), args[0]);
    self->set_label(*label);
  }
}

static void ChildNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value id(isolate, args[0]);
  float width = args.Length() > 1 ? static_cast<float>(args[1]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  float height = args.Length() > 2 ? static_cast<float>(args[2]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  ImGuiChildFlags child_flags =
      args.Length() > 3 ? static_cast<ImGuiChildFlags>(args[3]->Uint32Value(context).FromMaybe(0)) : 0;
  ImGuiWindowFlags window_flags =
      args.Length() > 4 ? static_cast<ImGuiWindowFlags>(args[4]->Uint32Value(context).FromMaybe(0)) : 0;
  new ChildWidget(env->principal_realm(), args.This(), *id, width, height, child_flags, window_flags);
}

static void GroupNew(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args.GetIsolate());
  new GroupWidget(env->principal_realm(), args.This());
}

static void DisabledNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  bool disabled = args.Length() > 0 ? args[0]->BooleanValue(isolate) : true;
  new DisabledWidget(env->principal_realm(), args.This(), disabled);
}

static void DisabledGetDisabled(const FunctionCallbackInfo<Value>& args) {
  DisabledWidget* self = BaseObject::Unwrap<DisabledWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->disabled());
}

static void DisabledSetDisabled(const FunctionCallbackInfo<Value>& args) {
  DisabledWidget* self = BaseObject::Unwrap<DisabledWidget>(args.This());
  if (self) self->set_disabled(args[0]->BooleanValue(args.GetIsolate()));
}

static void MenuBarNew(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args.GetIsolate());
  new MenuBarWidget(env->principal_realm(), args.This());
}

static void MenuNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Utf8Value label(isolate, args[0]);
  new MenuWidget(env->principal_realm(), args.This(), *label);
}

static void MenuGetLabel(const FunctionCallbackInfo<Value>& args) {
  MenuWidget* self = BaseObject::Unwrap<MenuWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->label().c_str()).ToLocalChecked());
  }
}

static void MenuSetLabel(const FunctionCallbackInfo<Value>& args) {
  MenuWidget* self = BaseObject::Unwrap<MenuWidget>(args.This());
  if (self) {
    Utf8Value label(args.GetIsolate(), args[0]);
    self->set_label(*label);
  }
}

static void MenuItemNew(const FunctionCallbackInfo<Value>& args) {
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

static void MenuItemGetClicked(const FunctionCallbackInfo<Value>& args) {
  MenuItemWidget* self = BaseObject::Unwrap<MenuItemWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->clicked());
}

static void MenuItemGetSelected(const FunctionCallbackInfo<Value>& args) {
  MenuItemWidget* self = BaseObject::Unwrap<MenuItemWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->selected());
}

static void MenuItemSetSelected(const FunctionCallbackInfo<Value>& args) {
  MenuItemWidget* self = BaseObject::Unwrap<MenuItemWidget>(args.This());
  if (self) self->set_selected(args[0]->BooleanValue(args.GetIsolate()));
}

static void MenuItemGetLabel(const FunctionCallbackInfo<Value>& args) {
  MenuItemWidget* self = BaseObject::Unwrap<MenuItemWidget>(args.This());
  if (self) {
    args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), self->label().c_str()).ToLocalChecked());
  }
}

static void MenuItemSetLabel(const FunctionCallbackInfo<Value>& args) {
  MenuItemWidget* self = BaseObject::Unwrap<MenuItemWidget>(args.This());
  if (self) {
    Utf8Value label(args.GetIsolate(), args[0]);
    self->set_label(*label);
  }
}

static void PopupNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Utf8Value id(isolate, args[0]);
  new PopupWidget(env->principal_realm(), args.This(), *id);
}

static void PopupOpen(const FunctionCallbackInfo<Value>& args) {
  PopupWidget* self = BaseObject::Unwrap<PopupWidget>(args.This());
  if (self) self->DoOpen();
}

static void PopupClose(const FunctionCallbackInfo<Value>& args) {
  PopupWidget* self = BaseObject::Unwrap<PopupWidget>(args.This());
  if (self) self->DoClose();
}

static void ModalNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  Utf8Value title(isolate, args[0]);
  new ModalWidget(env->principal_realm(), args.This(), *title);
}

static void ModalOpen(const FunctionCallbackInfo<Value>& args) {
  ModalWidget* self = BaseObject::Unwrap<ModalWidget>(args.This());
  if (self) self->DoOpen();
}

static void ModalClose(const FunctionCallbackInfo<Value>& args) {
  ModalWidget* self = BaseObject::Unwrap<ModalWidget>(args.This());
  if (self) self->DoClose();
}

static void ModalGetIsOpen(const FunctionCallbackInfo<Value>& args) {
  ModalWidget* self = BaseObject::Unwrap<ModalWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->is_open());
}

static void TooltipNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(isolate);
  std::string text;
  if (args.Length() > 0 && !args[0]->IsUndefined()) {
    Utf8Value t(isolate, args[0]);
    text = *t;
  }
  new TooltipWidget(env->principal_realm(), args.This(), text);
}

static void TableNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value id(isolate, args[0]);
  int columns = args[1]->Int32Value(context).FromMaybe(1);
  ImGuiTableFlags flags =
      args.Length() > 2 ? static_cast<ImGuiTableFlags>(args[2]->Uint32Value(context).FromMaybe(0)) : 0;
  new TableWidget(env->principal_realm(), args.This(), *id, columns, flags);
}

static void TableAddColumn(const FunctionCallbackInfo<Value>& args) {
  TableWidget* self = BaseObject::Unwrap<TableWidget>(args.This());
  if (self) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();
    Utf8Value name(isolate, args[0]);
    ImGuiTableColumnFlags flags =
        args.Length() > 1 ? static_cast<ImGuiTableColumnFlags>(args[1]->Uint32Value(context).FromMaybe(0)) : 0;
    float width = args.Length() > 2 ? static_cast<float>(args[2]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
    self->AddColumn(*name, flags, width);
  }
}

static void TableRowNew(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args.GetIsolate());
  new TableRowWidget(env->principal_realm(), args.This());
}

static std::vector<float> ArrayToFloatVector(Isolate* isolate, Local<Context> context, Local<Value> val) {
  std::vector<float> result;
  if (!val->IsArray()) return result;
  Local<v8::Array> arr = val.As<v8::Array>();
  for (uint32_t i = 0; i < arr->Length(); i++) {
    Local<Value> elem;
    if (arr->Get(context, i).ToLocal(&elem)) {
      result.push_back(static_cast<float>(elem->NumberValue(context).FromMaybe(0.0)));
    }
  }
  return result;
}

static void PlotLinesNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value label(isolate, args[0]);
  auto values = ArrayToFloatVector(isolate, context, args[1]);
  std::string overlay;
  if (args.Length() > 2 && !args[2]->IsUndefined()) {
    Utf8Value ov(isolate, args[2]);
    overlay = *ov;
  }
  float scale_min = args.Length() > 3 ? static_cast<float>(args[3]->NumberValue(context).FromMaybe(FLT_MAX)) : FLT_MAX;
  float scale_max = args.Length() > 4 ? static_cast<float>(args[4]->NumberValue(context).FromMaybe(FLT_MAX)) : FLT_MAX;
  float width = args.Length() > 5 ? static_cast<float>(args[5]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  float height = args.Length() > 6 ? static_cast<float>(args[6]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  new PlotLinesWidget(
      env->principal_realm(), args.This(), *label, std::move(values), overlay, scale_min, scale_max, width, height);
}

static void PlotLinesSetValues(const FunctionCallbackInfo<Value>& args) {
  PlotLinesWidget* self = BaseObject::Unwrap<PlotLinesWidget>(args.This());
  if (self) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();
    self->set_values(ArrayToFloatVector(isolate, context, args[0]));
  }
}

static void PlotHistogramNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Utf8Value label(isolate, args[0]);
  auto values = ArrayToFloatVector(isolate, context, args[1]);
  std::string overlay;
  if (args.Length() > 2 && !args[2]->IsUndefined()) {
    Utf8Value ov(isolate, args[2]);
    overlay = *ov;
  }
  float scale_min = args.Length() > 3 ? static_cast<float>(args[3]->NumberValue(context).FromMaybe(FLT_MAX)) : FLT_MAX;
  float scale_max = args.Length() > 4 ? static_cast<float>(args[4]->NumberValue(context).FromMaybe(FLT_MAX)) : FLT_MAX;
  float width = args.Length() > 5 ? static_cast<float>(args[5]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  float height = args.Length() > 6 ? static_cast<float>(args[6]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  new PlotHistogramWidget(
      env->principal_realm(), args.This(), *label, std::move(values), overlay, scale_min, scale_max, width, height);
}

static void PlotHistogramSetValues(const FunctionCallbackInfo<Value>& args) {
  PlotHistogramWidget* self = BaseObject::Unwrap<PlotHistogramWidget>(args.This());
  if (self) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();
    self->set_values(ArrayToFloatVector(isolate, context, args[0]));
  }
}

static void IndentNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  float width = args.Length() > 0 ? static_cast<float>(args[0]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  new IndentWidget(env->principal_realm(), args.This(), width);
}

static void UnindentNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  float width = args.Length() > 0 ? static_cast<float>(args[0]->NumberValue(context).FromMaybe(0.0)) : 0.0f;
  new UnindentWidget(env->principal_realm(), args.This(), width);
}

static void DummyNew(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  float width = static_cast<float>(args[0]->NumberValue(context).FromMaybe(0.0));
  float height = static_cast<float>(args[1]->NumberValue(context).FromMaybe(0.0));
  new DummyWidget(env->principal_realm(), args.This(), width, height);
}

static void CreatePerIsolateProperties(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, PanelNew);
    tmpl->SetClassName(OneByteString(isolate, "Panel"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetProperty(isolate, proto, "open", PanelGetOpen, PanelSetOpen);
    SetProperty(isolate, proto, "title", PanelGetTitle, PanelSetTitle);
    SetProperty(isolate, proto, "flags", PanelGetFlags, PanelSetFlags);
    target->Set(OneByteString(isolate, "Panel"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, TextNew);
    tmpl->SetClassName(OneByteString(isolate, "Text"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetProperty(isolate, proto, "text", TextGetText, TextSetText);
    target->Set(OneByteString(isolate, "Text"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, TextColoredNew);
    tmpl->SetClassName(OneByteString(isolate, "TextColored"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetProperty(isolate, proto, "text", TextGetText, TextSetText);
    target->Set(OneByteString(isolate, "TextColored"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, ButtonNew);
    tmpl->SetClassName(OneByteString(isolate, "Button"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetProperty(isolate, proto, "clicked", ButtonGetClicked, nullptr);
    SetProperty(isolate, proto, "label", ButtonGetLabel, ButtonSetLabel);
    target->Set(OneByteString(isolate, "Button"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, InvisibleButtonNew);
    tmpl->SetClassName(OneByteString(isolate, "InvisibleButton"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetProperty(isolate, proto, "clicked", InvisibleButtonGetClicked, nullptr);
    SetProperty(isolate, proto, "label", InvisibleButtonGetLabel, InvisibleButtonSetLabel);
    target->Set(OneByteString(isolate, "InvisibleButton"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, CheckboxNew);
    tmpl->SetClassName(OneByteString(isolate, "Checkbox"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetProperty(isolate, proto, "checked", CheckboxGetChecked, CheckboxSetChecked);
    SetProperty(isolate, proto, "label", CheckboxGetLabel, CheckboxSetLabel);
    target->Set(OneByteString(isolate, "Checkbox"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, SliderFloatNew);
    tmpl->SetClassName(OneByteString(isolate, "SliderFloat"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetProperty(isolate, proto, "value", SliderFloatGetValue, SliderFloatSetValue);
    SetProperty(isolate, proto, "label", SliderFloatGetLabel, SliderFloatSetLabel);
    target->Set(OneByteString(isolate, "SliderFloat"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, SliderIntNew);
    tmpl->SetClassName(OneByteString(isolate, "SliderInt"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetProperty(isolate, proto, "value", SliderIntGetValue, SliderIntSetValue);
    SetProperty(isolate, proto, "label", SliderIntGetLabel, SliderIntSetLabel);
    target->Set(OneByteString(isolate, "SliderInt"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, InputTextNew);
    tmpl->SetClassName(OneByteString(isolate, "InputText"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetProperty(isolate, proto, "text", InputTextGetText, InputTextSetText);
    SetProperty(isolate, proto, "label", InputTextGetLabel, InputTextSetLabel);
    target->Set(OneByteString(isolate, "InputText"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, SeparatorNew);
    tmpl->SetClassName(OneByteString(isolate, "Separator"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    InstallWidgetMethods(isolate, tmpl->PrototypeTemplate());
    target->Set(OneByteString(isolate, "Separator"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, SpacingNew);
    tmpl->SetClassName(OneByteString(isolate, "Spacing"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    InstallWidgetMethods(isolate, tmpl->PrototypeTemplate());
    target->Set(OneByteString(isolate, "Spacing"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, SameLineNew);
    tmpl->SetClassName(OneByteString(isolate, "SameLine"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    InstallWidgetMethods(isolate, tmpl->PrototypeTemplate());
    target->Set(OneByteString(isolate, "SameLine"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, TreeNodeNew);
    tmpl->SetClassName(OneByteString(isolate, "TreeNode"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetProperty(isolate, proto, "open", TreeNodeGetOpen, nullptr);
    SetProperty(isolate, proto, "label", TreeNodeGetLabel, TreeNodeSetLabel);
    target->Set(OneByteString(isolate, "TreeNode"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, CollapsingHeaderNew);
    tmpl->SetClassName(OneByteString(isolate, "CollapsingHeader"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetProperty(isolate, proto, "open", CollapsingHeaderGetOpen, nullptr);
    SetProperty(isolate, proto, "label", CollapsingHeaderGetLabel, CollapsingHeaderSetLabel);
    target->Set(OneByteString(isolate, "CollapsingHeader"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, TabBarNew);
    tmpl->SetClassName(OneByteString(isolate, "TabBar"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    InstallWidgetMethods(isolate, tmpl->PrototypeTemplate());
    target->Set(OneByteString(isolate, "TabBar"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, TabItemNew);
    tmpl->SetClassName(OneByteString(isolate, "TabItem"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetProperty(isolate, proto, "selected", TabItemGetSelected, nullptr);
    SetProperty(isolate, proto, "label", TabItemGetLabel, TabItemSetLabel);
    target->Set(OneByteString(isolate, "TabItem"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, DemoWindowNew);
    tmpl->SetClassName(OneByteString(isolate, "DemoWindow"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    InstallWidgetMethods(isolate, tmpl->PrototypeTemplate());
    target->Set(OneByteString(isolate, "DemoWindow"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, BulletTextNew);
    tmpl->SetClassName(OneByteString(isolate, "BulletText"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetProperty(isolate, proto, "text", TextGetText, TextSetText);
    target->Set(OneByteString(isolate, "BulletText"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, TextWrappedNew);
    tmpl->SetClassName(OneByteString(isolate, "TextWrapped"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetProperty(isolate, proto, "text", TextGetText, TextSetText);
    target->Set(OneByteString(isolate, "TextWrapped"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, TextDisabledNew);
    tmpl->SetClassName(OneByteString(isolate, "TextDisabled"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetProperty(isolate, proto, "text", TextGetText, TextSetText);
    target->Set(OneByteString(isolate, "TextDisabled"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, LabelTextNew);
    tmpl->SetClassName(OneByteString(isolate, "LabelText"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetProperty(isolate, proto, "text", TextGetText, TextSetText);
    SetProperty(isolate, proto, "label", LabelTextGetLabel, LabelTextSetLabel);
    target->Set(OneByteString(isolate, "LabelText"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, SeparatorTextNew);
    tmpl->SetClassName(OneByteString(isolate, "SeparatorText"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetProperty(isolate, proto, "text", TextGetText, TextSetText);
    target->Set(OneByteString(isolate, "SeparatorText"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, BulletNew);
    tmpl->SetClassName(OneByteString(isolate, "Bullet"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    InstallWidgetMethods(isolate, tmpl->PrototypeTemplate());
    target->Set(OneByteString(isolate, "Bullet"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, NewLineNew);
    tmpl->SetClassName(OneByteString(isolate, "NewLine"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    InstallWidgetMethods(isolate, tmpl->PrototypeTemplate());
    target->Set(OneByteString(isolate, "NewLine"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, SmallButtonNew);
    tmpl->SetClassName(OneByteString(isolate, "SmallButton"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetProperty(isolate, proto, "clicked", SmallButtonGetClicked, nullptr);
    SetProperty(isolate, proto, "label", SmallButtonGetLabel, SmallButtonSetLabel);
    target->Set(OneByteString(isolate, "SmallButton"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, ArrowButtonNew);
    tmpl->SetClassName(OneByteString(isolate, "ArrowButton"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetProperty(isolate, proto, "clicked", ArrowButtonGetClicked, nullptr);
    target->Set(OneByteString(isolate, "ArrowButton"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, RadioButtonNew);
    tmpl->SetClassName(OneByteString(isolate, "RadioButton"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetProperty(isolate, proto, "active", RadioButtonGetActive, RadioButtonSetActive);
    SetProperty(isolate, proto, "clicked", RadioButtonGetClicked, nullptr);
    SetProperty(isolate, proto, "label", RadioButtonGetLabel, RadioButtonSetLabel);
    target->Set(OneByteString(isolate, "RadioButton"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, DragFloatNew);
    tmpl->SetClassName(OneByteString(isolate, "DragFloat"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetProperty(isolate, proto, "value", DragFloatGetValue, DragFloatSetValue);
    SetProperty(isolate, proto, "label", DragFloatGetLabel, DragFloatSetLabel);
    target->Set(OneByteString(isolate, "DragFloat"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, DragIntNew);
    tmpl->SetClassName(OneByteString(isolate, "DragInt"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetProperty(isolate, proto, "value", DragIntGetValue, DragIntSetValue);
    SetProperty(isolate, proto, "label", DragIntGetLabel, DragIntSetLabel);
    target->Set(OneByteString(isolate, "DragInt"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, InputFloatNew);
    tmpl->SetClassName(OneByteString(isolate, "InputFloat"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetProperty(isolate, proto, "value", InputFloatGetValue, InputFloatSetValue);
    SetProperty(isolate, proto, "label", InputFloatGetLabel, InputFloatSetLabel);
    target->Set(OneByteString(isolate, "InputFloat"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, InputIntNew);
    tmpl->SetClassName(OneByteString(isolate, "InputInt"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetProperty(isolate, proto, "value", InputIntGetValue, InputIntSetValue);
    SetProperty(isolate, proto, "label", InputIntGetLabel, InputIntSetLabel);
    target->Set(OneByteString(isolate, "InputInt"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, InputTextMultilineNew);
    tmpl->SetClassName(OneByteString(isolate, "InputTextMultiline"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetProperty(isolate, proto, "text", InputTextMultilineGetText, InputTextMultilineSetText);
    SetProperty(isolate, proto, "label", InputTextMultilineGetLabel, InputTextMultilineSetLabel);
    target->Set(OneByteString(isolate, "InputTextMultiline"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, InputTextWithHintNew);
    tmpl->SetClassName(OneByteString(isolate, "InputTextWithHint"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetProperty(isolate, proto, "text", InputTextWithHintGetText, InputTextWithHintSetText);
    SetProperty(isolate, proto, "hint", InputTextWithHintGetHint, InputTextWithHintSetHint);
    SetProperty(isolate, proto, "label", InputTextWithHintGetLabel, InputTextWithHintSetLabel);
    target->Set(OneByteString(isolate, "InputTextWithHint"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, ColorEdit3New);
    tmpl->SetClassName(OneByteString(isolate, "ColorEdit3"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetProperty(isolate, proto, "r", ColorEdit3GetR, nullptr);
    SetProperty(isolate, proto, "g", ColorEdit3GetG, nullptr);
    SetProperty(isolate, proto, "b", ColorEdit3GetB, nullptr);
    SetProperty(isolate, proto, "label", ColorEdit3GetLabel, nullptr);
    target->Set(OneByteString(isolate, "ColorEdit3"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, ColorEdit4New);
    tmpl->SetClassName(OneByteString(isolate, "ColorEdit4"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetProperty(isolate, proto, "r", ColorEdit4GetR, nullptr);
    SetProperty(isolate, proto, "g", ColorEdit4GetG, nullptr);
    SetProperty(isolate, proto, "b", ColorEdit4GetB, nullptr);
    SetProperty(isolate, proto, "a", ColorEdit4GetA, nullptr);
    SetProperty(isolate, proto, "label", ColorEdit4GetLabel, nullptr);
    target->Set(OneByteString(isolate, "ColorEdit4"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, ColorPicker3New);
    tmpl->SetClassName(OneByteString(isolate, "ColorPicker3"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetProperty(isolate, proto, "r", ColorPicker3GetR, nullptr);
    SetProperty(isolate, proto, "g", ColorPicker3GetG, nullptr);
    SetProperty(isolate, proto, "b", ColorPicker3GetB, nullptr);
    SetProperty(isolate, proto, "label", ColorPicker3GetLabel, nullptr);
    target->Set(OneByteString(isolate, "ColorPicker3"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, ColorPicker4New);
    tmpl->SetClassName(OneByteString(isolate, "ColorPicker4"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetProperty(isolate, proto, "r", ColorPicker4GetR, nullptr);
    SetProperty(isolate, proto, "g", ColorPicker4GetG, nullptr);
    SetProperty(isolate, proto, "b", ColorPicker4GetB, nullptr);
    SetProperty(isolate, proto, "a", ColorPicker4GetA, nullptr);
    SetProperty(isolate, proto, "refR", ColorPicker4GetRefR, nullptr);
    SetProperty(isolate, proto, "refG", ColorPicker4GetRefG, nullptr);
    SetProperty(isolate, proto, "refB", ColorPicker4GetRefB, nullptr);
    SetProperty(isolate, proto, "refA", ColorPicker4GetRefA, nullptr);
    SetProperty(isolate, proto, "label", ColorPicker4GetLabel, nullptr);
    target->Set(OneByteString(isolate, "ColorPicker4"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, ColorButtonNew);
    tmpl->SetClassName(OneByteString(isolate, "ColorButton"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetProperty(isolate, proto, "r", ColorButtonGetR, nullptr);
    SetProperty(isolate, proto, "g", ColorButtonGetG, nullptr);
    SetProperty(isolate, proto, "b", ColorButtonGetB, nullptr);
    SetProperty(isolate, proto, "a", ColorButtonGetA, nullptr);
    SetProperty(isolate, proto, "sizeX", ColorButtonGetSizeX, nullptr);
    SetProperty(isolate, proto, "sizeY", ColorButtonGetSizeY, nullptr);
    SetProperty(isolate, proto, "clicked", ColorButtonGetClicked, nullptr);
    SetProperty(isolate, proto, "label", ColorButtonGetLabel, nullptr);
    target->Set(OneByteString(isolate, "ColorButton"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, ComboNew);
    tmpl->SetClassName(OneByteString(isolate, "Combo"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetProperty(isolate, proto, "selected", ComboGetSelected, ComboSetSelected);
    SetProperty(isolate, proto, "label", ComboGetLabel, nullptr);
    SetMethod(isolate, proto, "setItems", ComboSetItems);
    target->Set(OneByteString(isolate, "Combo"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, ListBoxNew);
    tmpl->SetClassName(OneByteString(isolate, "ListBox"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetProperty(isolate, proto, "selected", ListBoxGetSelected, ListBoxSetSelected);
    SetProperty(isolate, proto, "label", ListBoxGetLabel, nullptr);
    SetMethod(isolate, proto, "setItems", ListBoxSetItems);
    target->Set(OneByteString(isolate, "ListBox"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, ProgressBarNew);
    tmpl->SetClassName(OneByteString(isolate, "ProgressBar"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetProperty(isolate, proto, "fraction", ProgressBarGetFraction, ProgressBarSetFraction);
    SetProperty(isolate, proto, "overlay", ProgressBarGetOverlay, ProgressBarSetOverlay);
    target->Set(OneByteString(isolate, "ProgressBar"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, SelectableNew);
    tmpl->SetClassName(OneByteString(isolate, "Selectable"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetProperty(isolate, proto, "selected", SelectableGetSelected, SelectableSetSelected);
    SetProperty(isolate, proto, "label", SelectableGetLabel, SelectableSetLabel);
    target->Set(OneByteString(isolate, "Selectable"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, ChildNew);
    tmpl->SetClassName(OneByteString(isolate, "Child"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    InstallWidgetMethods(isolate, tmpl->PrototypeTemplate());
    target->Set(OneByteString(isolate, "Child"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, GroupNew);
    tmpl->SetClassName(OneByteString(isolate, "Group"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    InstallWidgetMethods(isolate, tmpl->PrototypeTemplate());
    target->Set(OneByteString(isolate, "Group"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, DisabledNew);
    tmpl->SetClassName(OneByteString(isolate, "Disabled"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetProperty(isolate, proto, "disabled", DisabledGetDisabled, DisabledSetDisabled);
    target->Set(OneByteString(isolate, "Disabled"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, MenuBarNew);
    tmpl->SetClassName(OneByteString(isolate, "MenuBar"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    InstallWidgetMethods(isolate, tmpl->PrototypeTemplate());
    target->Set(OneByteString(isolate, "MenuBar"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, MenuNew);
    tmpl->SetClassName(OneByteString(isolate, "Menu"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetProperty(isolate, proto, "label", MenuGetLabel, MenuSetLabel);
    target->Set(OneByteString(isolate, "Menu"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, MenuItemNew);
    tmpl->SetClassName(OneByteString(isolate, "MenuItem"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetProperty(isolate, proto, "clicked", MenuItemGetClicked, nullptr);
    SetProperty(isolate, proto, "selected", MenuItemGetSelected, MenuItemSetSelected);
    SetProperty(isolate, proto, "label", MenuItemGetLabel, MenuItemSetLabel);
    target->Set(OneByteString(isolate, "MenuItem"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, PopupNew);
    tmpl->SetClassName(OneByteString(isolate, "Popup"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetMethod(isolate, proto, "open", PopupOpen);
    SetMethod(isolate, proto, "close", PopupClose);
    target->Set(OneByteString(isolate, "Popup"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, ModalNew);
    tmpl->SetClassName(OneByteString(isolate, "Modal"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetMethod(isolate, proto, "open", ModalOpen);
    SetMethod(isolate, proto, "close", ModalClose);
    SetProperty(isolate, proto, "isOpen", ModalGetIsOpen, nullptr);
    target->Set(OneByteString(isolate, "Modal"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, TooltipNew);
    tmpl->SetClassName(OneByteString(isolate, "Tooltip"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetProperty(isolate, proto, "text", TextGetText, TextSetText);
    target->Set(OneByteString(isolate, "Tooltip"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, TableNew);
    tmpl->SetClassName(OneByteString(isolate, "Table"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetMethod(isolate, proto, "addColumn", TableAddColumn);
    target->Set(OneByteString(isolate, "Table"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, TableRowNew);
    tmpl->SetClassName(OneByteString(isolate, "TableRow"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    InstallWidgetMethods(isolate, tmpl->PrototypeTemplate());
    target->Set(OneByteString(isolate, "TableRow"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, PlotLinesNew);
    tmpl->SetClassName(OneByteString(isolate, "PlotLines"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetMethod(isolate, proto, "setValues", PlotLinesSetValues);
    target->Set(OneByteString(isolate, "PlotLines"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, PlotHistogramNew);
    tmpl->SetClassName(OneByteString(isolate, "PlotHistogram"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    Local<ObjectTemplate> proto = tmpl->PrototypeTemplate();
    InstallWidgetMethods(isolate, proto);
    SetMethod(isolate, proto, "setValues", PlotHistogramSetValues);
    target->Set(OneByteString(isolate, "PlotHistogram"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, IndentNew);
    tmpl->SetClassName(OneByteString(isolate, "Indent"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    InstallWidgetMethods(isolate, tmpl->PrototypeTemplate());
    target->Set(OneByteString(isolate, "Indent"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, UnindentNew);
    tmpl->SetClassName(OneByteString(isolate, "Unindent"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    InstallWidgetMethods(isolate, tmpl->PrototypeTemplate());
    target->Set(OneByteString(isolate, "Unindent"), tmpl);
  }

  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, DummyNew);
    tmpl->SetClassName(OneByteString(isolate, "Dummy"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    InstallWidgetMethods(isolate, tmpl->PrototypeTemplate());
    target->Set(OneByteString(isolate, "Dummy"), tmpl);
  }
}

static void CreatePerContextProperties(Local<Object> target, Local<Context> context) {}

NYX_BINDING_PER_ISOLATE_INIT(gui, CreatePerIsolateProperties)
NYX_BINDING_CONTEXT_AWARE(gui, CreatePerContextProperties)

}  // namespace nyx
