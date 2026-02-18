#include "nyx/gui/stack.h"

#include "nyx/env.h"

namespace nyx {

using v8::Array;
using v8::Boolean;
using v8::Context;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::HandleScope;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::ObjectTemplate;
using v8::Value;

void StackWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  SetProtoProperty(isolate, tmpl, "id", GetID, SetID);
  SetProtoProperty(isolate, tmpl, "clipRect", GetClipRect);
  SetProtoMethod(isolate, tmpl, "setClipRect", SetClipRect);
  SetProtoProperty(isolate, tmpl, "colors", GetColors);
  SetProtoMethod(isolate, tmpl, "setColor", SetColor);
  SetProtoProperty(isolate, tmpl, "vars", GetVars);
  SetProtoMethod(isolate, tmpl, "setVar", SetVar);
  SetProtoProperty(isolate, tmpl, "tabStop", GetTabStop, SetTabStop);
  SetProtoProperty(isolate, tmpl, "buttonRepeat", GetButtonRepeat, SetButtonRepeat);
  SetProtoProperty(isolate, tmpl, "itemWidth", GetItemWidth, SetItemWidth);
  SetProtoProperty(isolate, tmpl, "textWrap", GetTextWrap, SetTextWrap);

  tmpl->SetClassName(FixedOneByteString(isolate, "Stack"));
  target->Set(FixedOneByteString(isolate, "Stack"), tmpl);
}

void StackWidget::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  new StackWidget(env->principal_realm(), args.This());
}

void StackWidget::GetID(const FunctionCallbackInfo<Value>& args) {
  StackWidget* self = BaseObject::Unwrap<StackWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->id());
}

void StackWidget::SetID(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  StackWidget* self = BaseObject::Unwrap<StackWidget>(args.This());
  if (self) self->set_id(args[0]->NumberValue(context).FromMaybe(-1));
}

void StackWidget::GetClipRect(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  StackWidget* self = BaseObject::Unwrap<StackWidget>(args.This());
  if (self) {
    const StackWidget::ClipRectData& data = self->clip_rect();
    Local<Object> obj = Object::New(isolate);
    obj->Set(context, OneByteString(isolate, "min"), data.min.ToObject(context));
    obj->Set(context, OneByteString(isolate, "max"), data.max.ToObject(context));
    obj->Set(context,
             OneByteString(isolate, "intersectWithCurrentClipRect"),
             Boolean::New(isolate, data.intersect_with_current_clip_rect));
    args.GetReturnValue().Set(obj);
  }
}

void StackWidget::SetClipRect(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  StackWidget* self = BaseObject::Unwrap<StackWidget>(args.This());
  ImVec2 min = {
      static_cast<float>(args[0]->NumberValue(context).FromMaybe(-1.0f)),
      static_cast<float>(args[1]->NumberValue(context).FromMaybe(-1.0f)),
  };
  ImVec2 max = {
      static_cast<float>(args[2]->NumberValue(context).FromMaybe(-1.0f)),
      static_cast<float>(args[3]->NumberValue(context).FromMaybe(-1.0f)),
  };
  bool intersect_with_current_clip_rect = args[4]->BooleanValue(isolate);
  if (self) {
    self->set_clip_rect({min, max, intersect_with_current_clip_rect});
  }
}

void StackWidget::GetColors(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  StackWidget* self = BaseObject::Unwrap<StackWidget>(args.This());
  if (self) {
    const std::map<ImGuiCol, ImVec4>& colors = self->colors();
    Local<Array> arr = Array::New(isolate, static_cast<int>(colors.size()));
    int32_t index = 0;
    for (const auto& [idx, color] : colors) {
      Local<Object> obj = Object::New(isolate);
      obj->Set(context, OneByteString(isolate, "idx"), Number::New(isolate, idx));
      obj->Set(context, OneByteString(isolate, "r"), Number::New(isolate, color.x));
      obj->Set(context, OneByteString(isolate, "g"), Number::New(isolate, color.y));
      obj->Set(context, OneByteString(isolate, "b"), Number::New(isolate, color.z));
      obj->Set(context, OneByteString(isolate, "a"), Number::New(isolate, color.w));
      arr->Set(context, index++, obj);
    }
    args.GetReturnValue().Set(arr);
  }
}

void StackWidget::SetColor(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  StackWidget* self = BaseObject::Unwrap<StackWidget>(args.This());
  ImGuiCol idx = args[0]->NumberValue(context).FromMaybe(0);
  float r = static_cast<float>(args[1]->NumberValue(context).FromMaybe(0.0f));
  float g = static_cast<float>(args[2]->NumberValue(context).FromMaybe(0.0f));
  float b = static_cast<float>(args[3]->NumberValue(context).FromMaybe(0.0f));
  float a = args[4]->IsNumber() ? static_cast<float>(args[4]->NumberValue(context).FromJust()) : 1.0f;
  if (self) self->set_color(idx, {r, g, b, a});
}

void StackWidget::GetVars(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  StackWidget* self = BaseObject::Unwrap<StackWidget>(args.This());
  if (self) {
    const std::map<ImGuiStyleVar, ImVec2>& vars = self->vars();
    Local<Array> arr = Array::New(isolate, static_cast<int>(vars.size()));
    int32_t index = 0;
    for (const auto& [idx, value] : vars) {
      Local<Object> obj = Object::New(isolate);
      obj->Set(context, OneByteString(isolate, "idx"), Number::New(isolate, idx));
      obj->Set(context, OneByteString(isolate, "x"), Number::New(isolate, value.x));
      obj->Set(context, OneByteString(isolate, "y"), Number::New(isolate, value.y));
      arr->Set(context, index++, obj);
    }
    args.GetReturnValue().Set(arr);
  }
}

void StackWidget::SetVar(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  StackWidget* self = BaseObject::Unwrap<StackWidget>(args.This());
  ImGuiCol idx = args[0]->NumberValue(context).FromMaybe(0);
  float x = args[1]->NumberValue(context).FromMaybe(0.0f);
  float y = args[2]->NumberValue(context).FromMaybe(0.0f);
  if (self) self->set_var(idx, {x, y});
}

void StackWidget::GetTabStop(const FunctionCallbackInfo<Value>& args) {
  StackWidget* self = BaseObject::Unwrap<StackWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->tab_stop());
}

void StackWidget::SetTabStop(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  StackWidget* self = BaseObject::Unwrap<StackWidget>(args.This());
  if (self) self->set_tab_stop(args[0]->BooleanValue(isolate));
}

void StackWidget::GetButtonRepeat(const FunctionCallbackInfo<Value>& args) {
  StackWidget* self = BaseObject::Unwrap<StackWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->button_repeat());
}

void StackWidget::SetButtonRepeat(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  StackWidget* self = BaseObject::Unwrap<StackWidget>(args.This());
  if (self) self->set_button_repeat(args[0]->BooleanValue(isolate));
}

void StackWidget::GetItemWidth(const FunctionCallbackInfo<Value>& args) {
  StackWidget* self = BaseObject::Unwrap<StackWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->item_width());
}

void StackWidget::SetItemWidth(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  StackWidget* self = BaseObject::Unwrap<StackWidget>(args.This());
  if (self) self->set_item_width(args[0]->NumberValue(context).FromMaybe(0.0f));
}

void StackWidget::GetTextWrap(const FunctionCallbackInfo<Value>& args) {
  StackWidget* self = BaseObject::Unwrap<StackWidget>(args.This());
  if (self) args.GetReturnValue().Set(self->text_wrap());
}

void StackWidget::SetTextWrap(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  StackWidget* self = BaseObject::Unwrap<StackWidget>(args.This());
  if (self) self->set_text_wrap(args[0]->NumberValue(context).FromMaybe(0.0f));
}

void StackWidget::Render() {
  if (id_ != -1) {
    ImGui::PushID(id_);
  }
  if (clip_rect_data_.min.x != -1 && clip_rect_data_.min.y != -1 && clip_rect_data_.max.x != -1 &&
      clip_rect_data_.max.y != -1) {
    ImGui::PushClipRect(clip_rect_data_.min, clip_rect_data_.max, clip_rect_data_.intersect_with_current_clip_rect);
  }
  if (!colors_.empty()) {
    for (const auto& [idx, color] : colors_) {
      ImGui::PushStyleColor(idx, color);
    }
  }
  if (!vars_.empty()) {
    for (const auto& [idx, value] : vars_) {
      ImGui::PushStyleVar(idx, value);
    }
  }
  ImGui::PushTabStop(tab_stop_);
  ImGui::PushButtonRepeat(button_repeat_);
  if (item_width_ != 0.0f) {
    ImGui::PushItemWidth(item_width_);
  }
  if (text_wrap_ != 0.0f) {
    ImGui::PushTextWrapPos(text_wrap_);
  }
  RenderChildren();
  if (text_wrap_ != 0.0f) {
    ImGui::PopTextWrapPos();
  }
  if (item_width_ != 0.0f) {
    ImGui::PopItemWidth();
  }
  ImGui::PopButtonRepeat();
  ImGui::PopTabStop();
  if (!vars_.empty()) {
    ImGui::PopStyleVar(static_cast<int>(vars_.size()));
  }
  if (!colors_.empty()) {
    ImGui::PopStyleColor(static_cast<int>(colors_.size()));
  }
  if (clip_rect_data_.min.x != -1 && clip_rect_data_.min.y != -1 && clip_rect_data_.max.x != -1 &&
      clip_rect_data_.max.y != -1) {
    ImGui::PopClipRect();
  }
  if (id_ != -1) {
    ImGui::PopID();
  }
}

}  // namespace nyx
