#include "nyx/extension.h"

#include "nyx/base_object.h"
#include "nyx/isolate_data.h"

#include <fstream>
#include <sstream>
#include <unordered_map>

namespace nyx {

using v8::Local;
using v8::Object;
using v8::ObjectTemplate;

static std::vector<ExternalBinding> g_external_bindings;
static std::vector<ExternalBuiltin> g_external_builtins;
static std::vector<const BuiltinSourceMap*> g_external_source_maps;

static std::unordered_map<std::string, v8::Global<ObjectTemplate>> g_external_templates;

void RegisterBinding(const std::string& name, BindingInitCallback isolate_init, BindingContextCallback context_init) {
  g_external_bindings.push_back({name, isolate_init, context_init});
}

void RegisterBuiltin(const std::string& id, const std::string& source) {
  g_external_builtins.push_back({id, source});
}

void RegisterBuiltinFromFile(const std::string& id, const std::string& filepath) {
  std::ifstream file(filepath);
  if (!file.good()) {
    fprintf(stderr, "Warning: Could not load builtin from file: %s\n", filepath.c_str());
    return;
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  g_external_builtins.push_back({id, buffer.str()});
}

void RegisterBuiltinSourceMap(const BuiltinSourceMap* source_map) {
  g_external_source_maps.push_back(source_map);
}

const std::vector<ExternalBinding>& GetExternalBindings() {
  return g_external_bindings;
}

const std::vector<ExternalBuiltin>& GetExternalBuiltins() {
  return g_external_builtins;
}

const std::vector<const BuiltinSourceMap*>& GetExternalBuiltinSourceMaps() {
  return g_external_source_maps;
}

void CreateExternalBindingTemplates(IsolateData* isolate_data) {
  v8::Isolate* isolate = isolate_data->isolate();

  for (const auto& binding : g_external_bindings) {
    Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
    templ->SetInternalFieldCount(BaseObject::kInternalFieldCount);
    binding.isolate_init(isolate_data, templ);
    g_external_templates[binding.name].Reset(isolate, templ);
  }
}

void CleanupExternalBindings() {
  for (auto& pair : g_external_templates) {
    pair.second.Reset();
  }
  g_external_templates.clear();
}

const ExternalBinding* FindExternalBinding(const std::string& name) {
  for (const auto& binding : g_external_bindings) {
    if (binding.name == name) {
      return &binding;
    }
  }
  return nullptr;
}

Local<ObjectTemplate> GetExternalBindingTemplate(v8::Isolate* isolate, const std::string& name) {
  auto it = g_external_templates.find(name);
  if (it != g_external_templates.end()) {
    return it->second.Get(isolate);
  }
  return Local<ObjectTemplate>();
}

}  // namespace nyx
