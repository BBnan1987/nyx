#include "nyx/nyx_binding.h"

#include "nyx/base_object.h"
#include "nyx/errors.h"
#include "nyx/extension.h"
#include "nyx/isolate_data.h"
#include "nyx/realm.h"

namespace nyx {

// Forward declaration from extension.cc
v8::Local<v8::ObjectTemplate> GetExternalBindingTemplate(v8::Isolate* isolate, const std::string& name);

using v8::Context;
using v8::EscapableHandleScope;
using v8::FunctionCallbackInfo;
using v8::HandleScope;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::ObjectTemplate;
using v8::String;
using v8::Value;

#define NYX_BUILTIN_STANDARD_BINDINGS(V)                                                                               \
  V(builtins)                                                                                                          \
  V(console)                                                                                                           \
  V(module_wrap)                                                                                                       \
  V(fs)                                                                                                                \
  V(gui)                                                                                                               \
  V(memory)                                                                                                            \
  V(process)                                                                                                           \
  V(timers)

#define NYX_BUILTIN_BINDINGS(V) NYX_BUILTIN_STANDARD_BINDINGS(V)

#define V(name) void _register_##name();
NYX_BUILTIN_BINDINGS(V)
#undef V

#define V(name) void _register_isolate_##name(IsolateData* isolate_data, v8::Local<v8::ObjectTemplate> target);
NYX_BINDINGS_WITH_PER_ISOLATE_INIT(V)
#undef V

static InternalBinding* kInternalBindings;

static InternalBinding* FindModule(const char* name) {
  InternalBinding* current = kInternalBindings;
  while (current != nullptr) {
    if (strcmp(current->name, name) == 0) {
      return current;
    }
    current = current->link;
  }
  return nullptr;
}

void AddInternalBinding(InternalBinding* binding) {
  binding->link = kInternalBindings;
  kInternalBindings = binding;
}

void RegisterBuiltinBindings() {
  // Reset the linked list to avoid circular references on restart.
  // Each binding is a file-static struct reused across calls, so
  // re-inserting without clearing creates cycles.
  kInternalBindings = nullptr;

#define V(name) _register_##name();
  NYX_BUILTIN_BINDINGS(V)
#undef V
}

void CreateInternalBindingTemplates(IsolateData* isolate_data) {
#define V(name)                                                                                                        \
  do {                                                                                                                 \
    Local<ObjectTemplate> templ = ObjectTemplate::New(isolate_data->isolate());                                        \
    templ->SetInternalFieldCount(BaseObject::kInternalFieldCount);                                                     \
    _register_isolate_##name(isolate_data, templ);                                                                     \
    isolate_data->set_##name##_binding_template(templ);                                                                \
  } while (0);
  NYX_BINDINGS_WITH_PER_ISOLATE_INIT(V)
#undef V

  // initialize external binding templates
  CreateExternalBindingTemplates(isolate_data);
}

static Local<Object> GetInternalBindingExportObject(IsolateData* isolate_data,
                                                    const char* binding_name,
                                                    Local<Context> context) {
  Local<ObjectTemplate> templ;
#define V(name)                                                                                                        \
  if (strcmp(binding_name, #name) == 0) {                                                                              \
    templ = isolate_data->name##_binding_template();                                                                   \
  } else
  NYX_BINDINGS_WITH_PER_ISOLATE_INIT(V)
#undef V
  {
    // check for external bindings
    templ = GetExternalBindingTemplate(isolate_data->isolate(), binding_name);
    if (templ.IsEmpty()) {
      templ = isolate_data->binding_data_default_template();
    }
  }
  Local<Object> obj = templ->NewInstance(context).ToLocalChecked();
  return obj;
}

static Local<Object> InitInternalBinding(Realm* realm, InternalBinding* binding) {
  EscapableHandleScope scope(realm->isolate());
  Local<Context> context = realm->context();
  Local<Object> exports = GetInternalBindingExportObject(realm->isolate_data(), binding->name, context);
  binding->register_context(exports, context);
  return scope.Escape(exports);
}

void GetInternalBinding(const FunctionCallbackInfo<Value>& args) {
  Realm* realm = Realm::GetCurrent(args);
  Isolate* isolate = realm->isolate();
  HandleScope scope(isolate);

  Local<String> module = args[0].As<String>();
  String::Utf8Value module_v(isolate, module);
  Local<Object> exports;

  InternalBinding* mod = FindModule(*module_v);
  if (mod != nullptr) {
    exports = InitInternalBinding(realm, mod);
    realm->internal_bindings.insert(mod);
  } else {
    // check for external bindings
    const ExternalBinding* ext = FindExternalBinding(*module_v);
    if (ext != nullptr) {
      Local<Context> context = realm->context();
      exports = GetInternalBindingExportObject(realm->isolate_data(), *module_v, context);
      // call context init if provided
      if (ext->context_init) {
        ext->context_init(exports, context);
      }
    } else {
      THROW_ERR_INVALID_MODULE(isolate, *module_v);
      return;
    }
  }

  args.GetReturnValue().Set(exports);
}

}  // namespace nyx
