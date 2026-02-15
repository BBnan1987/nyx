#include "nyx/env.h"

#include <imgui.h>

namespace nyx {

using v8::Context;
using v8::EscapableHandleScope;
using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::ObjectTemplate;
using v8::Value;

static Local<Object> CreateImVec2Object(Local<Context> context, const ImVec2& v) {
  Isolate* isolate = context->GetIsolate();
  EscapableHandleScope scope(isolate);
  Local<Object> obj = Object::New(isolate);
  obj->Set(context, OneByteString(isolate, "x"), Number::New(isolate, v.x));
  obj->Set(context, OneByteString(isolate, "y"), Number::New(isolate, v.y));
  return scope.Escape(obj);
}

void CreatePerIsolatePropertiesIO(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  Local<ObjectTemplate> tmpl = ObjectTemplate::New(isolate);

  SetProperty(isolate, tmpl, "deltaTime", [](const FunctionCallbackInfo<Value>& args) {
    args.GetReturnValue().Set(ImGui::GetIO().DeltaTime);
  });

  SetProperty(isolate, tmpl, "displaySize", [](const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Environment* env = Environment::GetCurrent(isolate);
    Local<Context> context = env->context();
    args.GetReturnValue().Set(CreateImVec2Object(context, ImGui::GetIO().DisplaySize));
  });

  SetProperty(isolate, tmpl, "displayFramebufferScale", [](const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Environment* env = Environment::GetCurrent(isolate);
    Local<Context> context = env->context();
    args.GetReturnValue().Set(CreateImVec2Object(context, ImGui::GetIO().DisplayFramebufferScale));
  });

  SetProperty(isolate, tmpl, "mousePos", [](const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Environment* env = Environment::GetCurrent(isolate);
    Local<Context> context = env->context();
    args.GetReturnValue().Set(CreateImVec2Object(context, ImGui::GetIO().MousePos));
  });

  SetProperty(isolate, tmpl, "mouseWheel", [](const FunctionCallbackInfo<Value>& args) {
    args.GetReturnValue().Set(ImGui::GetIO().MouseWheel);
  });

  SetProperty(isolate, tmpl, "mouseWheelH", [](const FunctionCallbackInfo<Value>& args) {
    args.GetReturnValue().Set(ImGui::GetIO().MouseWheelH);
  });

  SetProperty(isolate, tmpl, "keyCtrl", [](const FunctionCallbackInfo<Value>& args) {
    args.GetReturnValue().Set(ImGui::GetIO().KeyCtrl);
  });

  SetProperty(isolate, tmpl, "keyShift", [](const FunctionCallbackInfo<Value>& args) {
    args.GetReturnValue().Set(ImGui::GetIO().KeyShift);
  });

  SetProperty(isolate, tmpl, "keyAlt", [](const FunctionCallbackInfo<Value>& args) {
    args.GetReturnValue().Set(ImGui::GetIO().KeyAlt);
  });

  SetProperty(isolate, tmpl, "keySuper", [](const FunctionCallbackInfo<Value>& args) {
    args.GetReturnValue().Set(ImGui::GetIO().KeySuper);
  });

  SetProperty(isolate, tmpl, "wantCaptureMouse", [](const FunctionCallbackInfo<Value>& args) {
    args.GetReturnValue().Set(ImGui::GetIO().WantCaptureMouse);
  });

  SetProperty(isolate, tmpl, "wantCaptureKeyboard", [](const FunctionCallbackInfo<Value>& args) {
    args.GetReturnValue().Set(ImGui::GetIO().WantCaptureKeyboard);
  });

  SetProperty(isolate, tmpl, "wantTextInput", [](const FunctionCallbackInfo<Value>& args) {
    args.GetReturnValue().Set(ImGui::GetIO().WantTextInput);
  });

  SetProperty(isolate, tmpl, "wantSetMousePos", [](const FunctionCallbackInfo<Value>& args) {
    args.GetReturnValue().Set(ImGui::GetIO().WantSetMousePos);
  });

  SetProperty(isolate, tmpl, "wantSaveIniSettings", [](const FunctionCallbackInfo<Value>& args) {
    args.GetReturnValue().Set(ImGui::GetIO().WantSaveIniSettings);
  });

  SetProperty(isolate, tmpl, "navActive", [](const FunctionCallbackInfo<Value>& args) {
    args.GetReturnValue().Set(ImGui::GetIO().NavActive);
  });

  SetProperty(isolate, tmpl, "navVisible", [](const FunctionCallbackInfo<Value>& args) {
    args.GetReturnValue().Set(ImGui::GetIO().NavVisible);
  });

  SetProperty(isolate, tmpl, "framerate", [](const FunctionCallbackInfo<Value>& args) {
    args.GetReturnValue().Set(ImGui::GetIO().Framerate);
  });

  SetProperty(isolate, tmpl, "metricsRenderVertices", [](const FunctionCallbackInfo<Value>& args) {
    args.GetReturnValue().Set(ImGui::GetIO().MetricsRenderVertices);
  });

  SetProperty(isolate, tmpl, "metricsRenderIndices", [](const FunctionCallbackInfo<Value>& args) {
    args.GetReturnValue().Set(ImGui::GetIO().MetricsRenderIndices);
  });

  SetProperty(isolate, tmpl, "metricsRenderWindows", [](const FunctionCallbackInfo<Value>& args) {
    args.GetReturnValue().Set(ImGui::GetIO().MetricsRenderWindows);
  });

  SetProperty(isolate, tmpl, "metricsActiveWindows", [](const FunctionCallbackInfo<Value>& args) {
    args.GetReturnValue().Set(ImGui::GetIO().MetricsActiveWindows);
  });

  SetProperty(
      isolate,
      tmpl,
      "fontGlobalScale",
      [](const FunctionCallbackInfo<Value>& args) { args.GetReturnValue().Set(ImGui::GetIO().FontGlobalScale); },
      [](const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        Environment* env = Environment::GetCurrent(isolate);
        Local<Context> context = env->context();
        ImGui::GetIO().FontGlobalScale = args[0]->NumberValue(context).FromMaybe(1.0f);
      });

  SetProperty(
      isolate,
      tmpl,
      "fontAllowUserScaling",
      [](const FunctionCallbackInfo<Value>& args) { args.GetReturnValue().Set(ImGui::GetIO().FontAllowUserScaling); },
      [](const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        ImGui::GetIO().FontAllowUserScaling = args[0]->IsBoolean() ? args[0]->BooleanValue(isolate) : false;
      });

  SetProperty(
      isolate,
      tmpl,
      "mouseDoubleClickTime",
      [](const FunctionCallbackInfo<Value>& args) { args.GetReturnValue().Set(ImGui::GetIO().MouseDoubleClickTime); },
      [](const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        Environment* env = Environment::GetCurrent(isolate);
        Local<Context> context = env->context();
        ImGui::GetIO().MouseDoubleClickTime = args[0]->NumberValue(context).FromMaybe(0.3f);
      });

  SetProperty(
      isolate,
      tmpl,
      "mouseDoubleClickMaxDist",
      [](const FunctionCallbackInfo<Value>& args) {
        args.GetReturnValue().Set(ImGui::GetIO().MouseDoubleClickMaxDist);
      },
      [](const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        Environment* env = Environment::GetCurrent(isolate);
        Local<Context> context = env->context();
        ImGui::GetIO().MouseDoubleClickMaxDist = args[0]->NumberValue(context).FromMaybe(6.0f);
      });

  SetProperty(
      isolate,
      tmpl,
      "keyRepeatDelay",
      [](const FunctionCallbackInfo<Value>& args) { args.GetReturnValue().Set(ImGui::GetIO().KeyRepeatDelay);
      },
      [](const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        Environment* env = Environment::GetCurrent(isolate);
        Local<Context> context = env->context();
        ImGui::GetIO().KeyRepeatDelay = args[0]->NumberValue(context).FromMaybe(0.275f);
      });

  SetProperty(
      isolate,
      tmpl,
      "keyRepeatRate",
      [](const FunctionCallbackInfo<Value>& args) { args.GetReturnValue().Set(ImGui::GetIO().KeyRepeatRate); },
      [](const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        Environment* env = Environment::GetCurrent(isolate);
        Local<Context> context = env->context();
        ImGui::GetIO().KeyRepeatRate = args[0]->NumberValue(context).FromMaybe(0.05f);
      });

  SetProperty(
      isolate,
      tmpl,
      "configFlags",
      [](const FunctionCallbackInfo<Value>& args) { args.GetReturnValue().Set(ImGui::GetIO().ConfigFlags); },
      [](const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        Environment* env = Environment::GetCurrent(isolate);
        Local<Context> context = env->context();
        ImGui::GetIO().ConfigFlags = args[0]->Uint32Value(context).FromMaybe(0);
      });

  SetProperty(
      isolate,
      tmpl,
      "backendFlags",
      [](const FunctionCallbackInfo<Value>& args) { args.GetReturnValue().Set(ImGui::GetIO().BackendFlags); },
      [](const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        Environment* env = Environment::GetCurrent(isolate);
        Local<Context> context = env->context();
        ImGui::GetIO().BackendFlags = args[0]->Uint32Value(context).FromMaybe(0);
      });

  target->Set(isolate, "io", tmpl);
}

}  // namespace nyx
