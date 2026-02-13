#pragma once

#include "nyx/util.h"

namespace nyx {

class IsolateData;

#define NYX_BINDINGS_WITH_PER_ISOLATE_INIT(V)                                                                          \
  V(builtins)                                                                                                          \
  V(console)                                                                                                           \
  V(module_wrap)                                                                                                       \
  V(fs)                                                                                                                \
  V(process)                                                                                                           \
  V(memory)                                                                                                            \
  V(timers)                                                                                                            \
  V(gui)

typedef void (*BindingRegisterContextCallback)(v8::Local<v8::Object> target, v8::Local<v8::Context> context);

struct InternalBinding {
  const char* filename;
  const char* name;
  BindingRegisterContextCallback register_context;
  struct InternalBinding* link;
};
void AddInternalBinding(InternalBinding* binding);

#define NYX_BINDING_CONTEXT_AWARE(bindingname, func)                                                                   \
  static InternalBinding _binding = {                                                                                  \
      __FILE__,                                                                                                        \
      NYX_STRINGIFY(bindingname),                                                                                      \
      (BindingRegisterContextCallback)(func),                                                                          \
      nullptr,                                                                                                         \
  };                                                                                                                   \
  void _register_##bindingname() {                                                                                     \
    AddInternalBinding(&_binding);                                                                                     \
  }

#define NYX_BINDING_PER_ISOLATE_INIT(name, func)                                                                       \
  void _register_isolate_##name(IsolateData* isolate_data, v8::Local<v8::ObjectTemplate> target) {                     \
    func(isolate_data, target);                                                                                        \
  }

void RegisterBuiltinBindings();
void CreateInternalBindingTemplates(IsolateData* isolate_data);
void GetInternalBinding(const v8::FunctionCallbackInfo<v8::Value>& args);

}  // namespace nyx
