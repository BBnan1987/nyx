#include "nyx/isolate_data.h"

#include "nyx/base_object.h"
#include "nyx/extension.h"

namespace nyx {

using v8::HandleScope;
using v8::Isolate;
using v8::Local;
using v8::NewStringType;
using v8::ObjectTemplate;
using v8::Private;
using v8::String;
using v8::Symbol;

IsolateData::IsolateData(Isolate* isolate, uv_loop_t* event_loop) : isolate_(isolate), event_loop_(event_loop) {
  CreateProperties();
}

IsolateData::~IsolateData() {
  CleanupExternalBindings();
}

#define VP(PropertyName, StringValue) V(v8::Private, PropertyName)
#define VY(PropertyName, StringValue) V(v8::Symbol, PropertyName)
#define VS(PropertyName, StringValue) V(v8::String, PropertyName)
#define VR(PropertyName, TypeName) V(v8::Private, per_realm_##PropertyName)
#define V(TypeName, PropertyName)                                                                                      \
  v8::Local<TypeName> IsolateData::PropertyName() const {                                                              \
    return PropertyName##_.Get(isolate_);                                                                              \
  }
PER_ISOLATE_PRIVATE_SYMBOL_PROPERTIES(VP)
PER_ISOLATE_SYMBOL_PROPERTIES(VY)
PER_ISOLATE_STRING_PROPERTIES(VS)
PER_REALM_STRONG_PERSISTENT_VALUES(VR)
#undef V
#undef VS
#undef VY
#undef VP

// Template property accessors
#define VM(PropertyName) V(PropertyName##_binding_template, v8::ObjectTemplate)
#define V(PropertyName, TypeName)                                                                                      \
  void IsolateData::set_##PropertyName(v8::Local<TypeName> value) {                                                    \
    PropertyName##_.Set(isolate_, value);                                                                              \
  }                                                                                                                    \
  v8::Local<TypeName> IsolateData::PropertyName() const {                                                              \
    return PropertyName##_.Get(isolate_);                                                                              \
  }
PER_ISOLATE_TEMPLATE_PROPERTIES(V)
NYX_BINDINGS_WITH_PER_ISOLATE_INIT(VM)
#undef V
#undef VM

void IsolateData::CreateProperties() {
  Isolate::Scope isolate_scope(isolate_);
  HandleScope handle_scope(isolate_);

#define V(PropertyName, StringValue)                                                                                   \
  PropertyName##_.Set(isolate_,                                                                                        \
                      Private::New(isolate_,                                                                           \
                                   String::NewFromOneByte(isolate_,                                                    \
                                                          reinterpret_cast<const uint8_t*>(StringValue),               \
                                                          NewStringType::kInternalized,                                \
                                                          sizeof(StringValue) - 1)                                     \
                                       .ToLocalChecked()));
  PER_ISOLATE_PRIVATE_SYMBOL_PROPERTIES(V)
#undef V

#define V(PropertyName, StringValue)                                                                                   \
  PropertyName##_.Set(isolate_,                                                                                        \
                      Symbol::New(isolate_,                                                                            \
                                  String::NewFromOneByte(isolate_,                                                     \
                                                         reinterpret_cast<const uint8_t*>(StringValue),                \
                                                         NewStringType::kInternalized,                                 \
                                                         sizeof(StringValue) - 1)                                      \
                                      .ToLocalChecked()));
  PER_ISOLATE_SYMBOL_PROPERTIES(V)
#undef V

#define V(PropertyName, StringValue)                                                                                   \
  PropertyName##_.Set(isolate_,                                                                                        \
                      String::NewFromOneByte(isolate_,                                                                 \
                                             reinterpret_cast<const uint8_t*>(StringValue),                            \
                                             NewStringType::kInternalized,                                             \
                                             sizeof(StringValue) - 1)                                                  \
                          .ToLocalChecked());
  PER_ISOLATE_STRING_PROPERTIES(V)
#undef V
#define V(PropertyName, TypeName)                                                                                      \
  per_realm_##PropertyName##_.Set(                                                                                     \
      isolate_,                                                                                                        \
      Private::New(isolate_,                                                                                           \
                   String::NewFromOneByte(isolate_,                                                                    \
                                          reinterpret_cast<const uint8_t*>("per_realm_" #PropertyName),                \
                                          NewStringType::kInternalized,                                                \
                                          sizeof("per_realm_" #PropertyName) - 1)                                      \
                       .ToLocalChecked()));
  PER_REALM_STRONG_PERSISTENT_VALUES(V)
#undef V

  Local<ObjectTemplate> templ = ObjectTemplate::New(isolate());
  templ->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  set_binding_data_default_template(templ);
  CreateInternalBindingTemplates(this);
}

}  // namespace nyx
