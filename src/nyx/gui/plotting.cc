#include "nyx/gui/plotting.h"

#include "nyx/env.h"

namespace nyx {

using v8::Array;
using v8::Context;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::ObjectTemplate;
using v8::String;
using v8::Value;

PlotLinesWidget::PlotLinesWidget(Realm* realm,
                                 Local<Object> object,
                                 const std::string& label,
                                 std::vector<float> values,
                                 const std::string& overlay,
                                 float scale_min,
                                 float scale_max,
                                 float width,
                                 float height)
    : Widget(realm, object, label),
      values_(std::move(values)),
      overlay_(overlay),
      scale_min_(scale_min),
      scale_max_(scale_max),
      width_(width),
      height_(height) {}

void PlotLinesWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  SetProtoMethod(isolate, tmpl, "setValues", SetValues);

  tmpl->SetClassName(FixedOneByteString(isolate, "PlotLines"));
  target->Set(FixedOneByteString(isolate, "PlotLines"), tmpl);
}

static std::vector<float> ArrayToFloatVector(Isolate* isolate, Local<Context> context, Local<Value> val) {
  std::vector<float> result;
  if (!val->IsArray()) return result;
  Local<Array> arr = val.As<Array>();
  for (uint32_t i = 0; i < arr->Length(); i++) {
    Local<Value> elem;
    if (arr->Get(context, i).ToLocal(&elem)) {
      result.push_back(static_cast<float>(elem->NumberValue(context).FromMaybe(0.0)));
    }
  }
  return result;
}

void PlotLinesWidget::New(const FunctionCallbackInfo<Value>& args) {
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

void PlotLinesWidget::SetValues(const FunctionCallbackInfo<Value>& args) {
  PlotLinesWidget* self = BaseObject::Unwrap<PlotLinesWidget>(args.This());
  if (self) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();
    self->set_values(ArrayToFloatVector(isolate, context, args[0]));
  }
}

void PlotLinesWidget::Render() {
  ImGui::PlotLines(label_.c_str(),
                   values_.data(),
                   static_cast<int>(values_.size()),
                   0,
                   overlay_.empty() ? nullptr : overlay_.c_str(),
                   scale_min_,
                   scale_max_,
                   ImVec2(width_, height_));
}

PlotHistogramWidget::PlotHistogramWidget(Realm* realm,
                                         Local<Object> object,
                                         const std::string& label,
                                         std::vector<float> values,
                                         const std::string& overlay,
                                         float scale_min,
                                         float scale_max,
                                         float width,
                                         float height)
    : Widget(realm, object, label),
      values_(std::move(values)),
      overlay_(overlay),
      scale_min_(scale_min),
      scale_max_(scale_max),
      width_(width),
      height_(height) {}

void PlotHistogramWidget::Initialize(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  tmpl->Inherit(Widget::GetConstructorTemplate(isolate_data));

  SetProtoMethod(isolate, tmpl, "setValues", SetValues);

  tmpl->SetClassName(FixedOneByteString(isolate, "PlotHistogram"));
  target->Set(FixedOneByteString(isolate, "PlotHistogram"), tmpl);
}

void PlotHistogramWidget::New(const FunctionCallbackInfo<Value>& args) {
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

void PlotHistogramWidget::SetValues(const FunctionCallbackInfo<Value>& args) {
  PlotHistogramWidget* self = BaseObject::Unwrap<PlotHistogramWidget>(args.This());
  if (self) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();
    self->set_values(ArrayToFloatVector(isolate, context, args[0]));
  }
}

void PlotHistogramWidget::Render() {
  ImGui::PlotHistogram(label_.c_str(),
                       values_.data(),
                       static_cast<int>(values_.size()),
                       0,
                       overlay_.empty() ? nullptr : overlay_.c_str(),
                       scale_min_,
                       scale_max_,
                       ImVec2(width_, height_));
}

}  // namespace nyx
