#pragma once

#include <uv.h>

#include "nyx/nyx_binding.h"
#include "nyx/util.h"

namespace nyx {

#define PER_ISOLATE_PRIVATE_SYMBOL_PROPERTIES(V)                                                                       \
  V(host_defined_option_symbol, "nyx:host_defined_option_symbol")                                                      \
  V(source_map_data_private_symbol, "nyx:source_map_data_private_symbol")

#define PER_ISOLATE_SYMBOL_PROPERTIES(V) V(iterator_symbol, "Symbol.iterator")

#define PER_ISOLATE_STRING_PROPERTIES(V)                                                                               \
  V(attributes_string, "attributes")                                                                                   \
  V(original_string, "original")                                                                                       \
  V(required_module_facade_url_string, "nyx:internal/required_module_default_facade")                                  \
  V(required_module_facade_source_string,                                                                              \
    "export * from 'original';"                                                                                        \
    "export { default } from 'original';"                                                                              \
    "export const __esModule = true;")                                                                                 \
  V(source_map_url_string, "sourceMapURL")                                                                             \
  V(source_url_string, "sourceURL")                                                                                    \
  V(specifier_string, "specifier")                                                                                     \
  V(synthetic_string, "synthetic")                                                                                     \
  V(url_string, "url")                                                                                                 \
  V(x_string, "x")                                                                                                     \
  V(y_string, "y")                                                                                                     \
  V(z_string, "z")                                                                                                     \
  V(w_string, "w")                                                                                                     \
  V(r_string, "r")                                                                                                     \
  V(g_string, "g")                                                                                                     \
  V(b_string, "b")                                                                                                     \
  V(a_string, "a")

#define PER_ISOLATE_TEMPLATE_PROPERTIES(V)                                                                             \
  V(binding_data_default_template, v8::ObjectTemplate)                                                                 \
  V(module_wrap_constructor_template, v8::FunctionTemplate)                                                            \
  V(widget_constructor_template, v8::FunctionTemplate)

#define PER_REALM_STRONG_PERSISTENT_VALUES(V)                                                                          \
  V(builtin_module_require, v8::Function)                                                                              \
  V(host_import_module_dynamically_callback, v8::Function)                                                             \
  V(host_initialize_import_meta_object_callback, v8::Function)                                                         \
  V(internal_binding_loader, v8::Function)

class IsolateData {
 public:
  IsolateData(v8::Isolate* isolate, uv_loop_t* event_loop);
  ~IsolateData();

  v8::Isolate* isolate() const { return isolate_; }
  uv_loop_t* event_loop() const { return event_loop_; }

#define VP(PropertyName, StringValue) V(v8::Private, PropertyName)
#define VY(PropertyName, StringValue) V(v8::Symbol, PropertyName)
#define VS(PropertyName, StringValue) V(v8::String, PropertyName)
#define VR(PropertyName, TypeName) V(v8::Private, per_realm_##PropertyName)
#define V(TypeName, PropertyName) v8::Local<TypeName> PropertyName() const;
  PER_ISOLATE_PRIVATE_SYMBOL_PROPERTIES(VP)
  PER_ISOLATE_SYMBOL_PROPERTIES(VY)
  PER_ISOLATE_STRING_PROPERTIES(VS)
  PER_REALM_STRONG_PERSISTENT_VALUES(VR)
#undef V
#undef VS
#undef VY
#undef VP

#define VM(PropertyName) V(PropertyName##_binding_template, v8::ObjectTemplate)
#define V(PropertyName, TypeName)                                                                                      \
  void set_##PropertyName(v8::Local<TypeName> value);                                                                  \
  v8::Local<TypeName> PropertyName() const;
  PER_ISOLATE_TEMPLATE_PROPERTIES(V)
  NYX_BINDINGS_WITH_PER_ISOLATE_INIT(VM)
#undef V
#undef VM

 private:
  void CreateProperties();

  v8::Isolate* isolate_;
  uv_loop_t* event_loop_;

#define VP(PropertyName, StringValue) V(v8::Private, PropertyName)
#define VY(PropertyName, StringValue) V(v8::Symbol, PropertyName)
#define VS(PropertyName, StringValue) V(v8::String, PropertyName)
#define VT(PropertyName, TypeName) V(TypeName, PropertyName)
#define VR(PropertyName, TypeName) V(v8::Private, per_realm_##PropertyName)
#define VM(PropertyName) V(v8::ObjectTemplate, PropertyName##_binding_template)
#define V(TypeName, PropertyName) v8::Eternal<TypeName> PropertyName##_;
  PER_ISOLATE_PRIVATE_SYMBOL_PROPERTIES(VP)
  PER_ISOLATE_SYMBOL_PROPERTIES(VY)
  PER_ISOLATE_STRING_PROPERTIES(VS)
  PER_ISOLATE_TEMPLATE_PROPERTIES(VT)
  PER_REALM_STRONG_PERSISTENT_VALUES(VR)
  NYX_BINDINGS_WITH_PER_ISOLATE_INIT(VM)
#undef V
#undef VM
#undef VT
#undef VS
#undef VY
#undef VP
};

}  // namespace nyx
