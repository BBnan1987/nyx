#pragma once

#define NYX_IMGUI_V8_INTEGRATION

#include <imgui.h>
#include <v8.h>

namespace nyx {

// Helper function to get a float from a JS object property
inline float GetFloatProperty(v8::Isolate* isolate, v8::Local<v8::Object> obj, const char* key, float default_value) {
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  v8::Local<v8::Value> val;
  if (obj->Get(context, v8::String::NewFromUtf8(isolate, key).ToLocalChecked()).ToLocal(&val) && val->IsNumber()) {
    return static_cast<float>(val.As<v8::Number>()->Value());
  }
  return default_value;
}

}  // namespace nyx
