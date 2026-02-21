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
    args.GetReturnValue().Set(ImGui::GetIO().DisplaySize.ToObject(context));
  });

  SetProperty(isolate, tmpl, "displayFramebufferScale", [](const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Environment* env = Environment::GetCurrent(isolate);
    Local<Context> context = env->context();
    args.GetReturnValue().Set(ImGui::GetIO().DisplayFramebufferScale.ToObject(context));
  });

  SetProperty(isolate, tmpl, "mousePos", [](const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Environment* env = Environment::GetCurrent(isolate);
    Local<Context> context = env->context();
    args.GetReturnValue().Set(ImGui::GetIO().MousePos.ToObject(context));
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
      [](const FunctionCallbackInfo<Value>& args) { args.GetReturnValue().Set(ImGui::GetIO().KeyRepeatDelay); },
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

  SetProperty(
      isolate,
      tmpl,
      "configInputTrickleEventQueue",
      [](const FunctionCallbackInfo<Value>& args) {
        args.GetReturnValue().Set(ImGui::GetIO().ConfigInputTrickleEventQueue);
      },
      [](const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        ImGui::GetIO().ConfigInputTrickleEventQueue = args[0]->BooleanValue(isolate);
      });

  SetProperty(
      isolate,
      tmpl,
      "mouseDrawCursor",
      [](const FunctionCallbackInfo<Value>& args) { args.GetReturnValue().Set(ImGui::GetIO().MouseDrawCursor); },
      [](const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        ImGui::GetIO().MouseDrawCursor = args[0]->BooleanValue(isolate);
      });

  SetProperty(
      isolate,
      tmpl,
      "configInputTextCursorBlink",
      [](const FunctionCallbackInfo<Value>& args) {
        args.GetReturnValue().Set(ImGui::GetIO().ConfigInputTextCursorBlink);
      },
      [](const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        ImGui::GetIO().ConfigInputTextCursorBlink = args[0]->BooleanValue(isolate);
      });

  SetProperty(
      isolate,
      tmpl,
      "configInputTextEnterKeepActive",
      [](const FunctionCallbackInfo<Value>& args) {
        args.GetReturnValue().Set(ImGui::GetIO().ConfigInputTextEnterKeepActive);
      },
      [](const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        ImGui::GetIO().ConfigInputTextEnterKeepActive = args[0]->BooleanValue(isolate);
      });

  SetProperty(
      isolate,
      tmpl,
      "configDragClickToInputText",
      [](const FunctionCallbackInfo<Value>& args) {
        args.GetReturnValue().Set(ImGui::GetIO().ConfigDragClickToInputText);
      },
      [](const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        ImGui::GetIO().ConfigDragClickToInputText = args[0]->BooleanValue(isolate);
      });

  SetProperty(
      isolate,
      tmpl,
      "configWindowsResizeFromEdges",
      [](const FunctionCallbackInfo<Value>& args) {
        args.GetReturnValue().Set(ImGui::GetIO().ConfigWindowsResizeFromEdges);
      },
      [](const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        ImGui::GetIO().ConfigWindowsResizeFromEdges = args[0]->BooleanValue(isolate);
      });

  SetProperty(
      isolate,
      tmpl,
      "configWindowsMoveFromTitleBarOnly",
      [](const FunctionCallbackInfo<Value>& args) {
        args.GetReturnValue().Set(ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly);
      },
      [](const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = args[0]->BooleanValue(isolate);
      });

  SetProperty(
      isolate,
      tmpl,
      "configMacOSXBehaviors",
      [](const FunctionCallbackInfo<Value>& args) { args.GetReturnValue().Set(ImGui::GetIO().ConfigMacOSXBehaviors); },
      [](const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        ImGui::GetIO().ConfigMacOSXBehaviors = args[0]->BooleanValue(isolate);
      });

  SetProperty(
      isolate,
      tmpl,
      "configDebugIsDebuggerPresent",
      [](const FunctionCallbackInfo<Value>& args) {
        args.GetReturnValue().Set(ImGui::GetIO().ConfigDebugIsDebuggerPresent);
      },
      [](const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        ImGui::GetIO().ConfigDebugIsDebuggerPresent = args[0]->BooleanValue(isolate);
      });

  SetProperty(
      isolate,
      tmpl,
      "configDebugBeginReturnValueOnce",
      [](const FunctionCallbackInfo<Value>& args) {
        args.GetReturnValue().Set(ImGui::GetIO().ConfigDebugBeginReturnValueOnce);
      },
      [](const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        ImGui::GetIO().ConfigDebugBeginReturnValueOnce = args[0]->BooleanValue(isolate);
      });

  SetProperty(
      isolate,
      tmpl,
      "configDebugBeginReturnValueLoop",
      [](const FunctionCallbackInfo<Value>& args) {
        args.GetReturnValue().Set(ImGui::GetIO().ConfigDebugBeginReturnValueLoop);
      },
      [](const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        ImGui::GetIO().ConfigDebugBeginReturnValueLoop = args[0]->BooleanValue(isolate);
      });

  SetProperty(
      isolate,
      tmpl,
      "configDebugIgnoreFocusLoss",
      [](const FunctionCallbackInfo<Value>& args) {
        args.GetReturnValue().Set(ImGui::GetIO().ConfigDebugIgnoreFocusLoss);
      },
      [](const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        ImGui::GetIO().ConfigDebugIgnoreFocusLoss = args[0]->BooleanValue(isolate);
      });

  SetProperty(
      isolate,
      tmpl,
      "configDebugIniSettings",
      [](const FunctionCallbackInfo<Value>& args) { args.GetReturnValue().Set(ImGui::GetIO().ConfigDebugIniSettings); },
      [](const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        ImGui::GetIO().ConfigDebugIniSettings = args[0]->BooleanValue(isolate);
      });

  SetMethod(isolate, tmpl, "isKeyDown", [](const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Environment* env = Environment::GetCurrent(isolate);
    Local<Context> context = env->context();
    if (args.Length() < 1) { args.GetReturnValue().Set(false); return; }
    int key = args[0]->Int32Value(context).FromMaybe(0);
    args.GetReturnValue().Set(ImGui::IsKeyDown(static_cast<ImGuiKey>(key)));
  });

  SetMethod(isolate, tmpl, "isKeyPressed", [](const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Environment* env = Environment::GetCurrent(isolate);
    Local<Context> context = env->context();
    if (args.Length() < 1) { args.GetReturnValue().Set(false); return; }
    int key = args[0]->Int32Value(context).FromMaybe(0);
    bool repeat = args.Length() < 2 ? true : args[1]->BooleanValue(isolate);
    args.GetReturnValue().Set(ImGui::IsKeyPressed(static_cast<ImGuiKey>(key), repeat));
  });

  SetMethod(isolate, tmpl, "isKeyReleased", [](const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Environment* env = Environment::GetCurrent(isolate);
    Local<Context> context = env->context();
    if (args.Length() < 1) { args.GetReturnValue().Set(false); return; }
    int key = args[0]->Int32Value(context).FromMaybe(0);
    args.GetReturnValue().Set(ImGui::IsKeyReleased(static_cast<ImGuiKey>(key)));
  });

  SetMethod(isolate, tmpl, "isMouseDown", [](const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Environment* env = Environment::GetCurrent(isolate);
    Local<Context> context = env->context();
    if (args.Length() < 1) { args.GetReturnValue().Set(false); return; }
    int btn = args[0]->Int32Value(context).FromMaybe(0);
    args.GetReturnValue().Set(ImGui::IsMouseDown(static_cast<ImGuiMouseButton>(btn)));
  });

  SetMethod(isolate, tmpl, "isMouseClicked", [](const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Environment* env = Environment::GetCurrent(isolate);
    Local<Context> context = env->context();
    if (args.Length() < 1) { args.GetReturnValue().Set(false); return; }
    int btn = args[0]->Int32Value(context).FromMaybe(0);
    bool repeat = args.Length() < 2 ? false : args[1]->BooleanValue(isolate);
    args.GetReturnValue().Set(ImGui::IsMouseClicked(static_cast<ImGuiMouseButton>(btn), repeat));
  });

  SetMethod(isolate, tmpl, "isMouseReleased", [](const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Environment* env = Environment::GetCurrent(isolate);
    Local<Context> context = env->context();
    if (args.Length() < 1) { args.GetReturnValue().Set(false); return; }
    int btn = args[0]->Int32Value(context).FromMaybe(0);
    args.GetReturnValue().Set(ImGui::IsMouseReleased(static_cast<ImGuiMouseButton>(btn)));
  });

  SetMethod(isolate, tmpl, "isMouseDoubleClicked", [](const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Environment* env = Environment::GetCurrent(isolate);
    Local<Context> context = env->context();
    if (args.Length() < 1) { args.GetReturnValue().Set(false); return; }
    int btn = args[0]->Int32Value(context).FromMaybe(0);
    args.GetReturnValue().Set(ImGui::IsMouseDoubleClicked(static_cast<ImGuiMouseButton>(btn)));
  });

  target->Set(isolate, "io", tmpl);
}

}  // namespace nyx
