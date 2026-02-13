#pragma once

#include <nyx/extension.h>

namespace dolos {

void InitDolosBinding(nyx::IsolateData* isolate_data, v8::Local<v8::ObjectTemplate> target);
void RegisterDolosBindingAndBuiltins();

}  // namespace dolos
