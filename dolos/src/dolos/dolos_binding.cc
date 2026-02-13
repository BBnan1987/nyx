#include "dolos/dolos_binding.h"

#include "dolos/offset_registry.h"

#include <nyx/extension.h>
#include <nyx/isolate_data.h>

namespace dolos {

using v8::BigInt;
using v8::Isolate;
using v8::Local;
using v8::ObjectTemplate;
using v8::PropertyAttribute;

void InitDolosBinding(nyx::IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();

  for (const auto& [name, address] : GetRegisteredOffsets()) {
    target->Set(
        isolate, name.c_str(), BigInt::New(isolate, reinterpret_cast<int64_t>(address)), PropertyAttribute::ReadOnly);
  }
}

}  // namespace dolos
