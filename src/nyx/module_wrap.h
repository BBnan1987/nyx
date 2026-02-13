#pragma once

#include "nyx/base_object.h"
#include "nyx/isolate_data.h"

#include <string>
#include <unordered_map>

namespace nyx {

struct ModuleCacheKey {
  using ImportAttributeVector = std::vector<std::pair<std::string, std::string>>;

  std::string specifier;
  ImportAttributeVector import_attributes;
  std::size_t hash;

  std::string ToString() const;

  template <int elements_per_attribute = 3>
  static ModuleCacheKey From(v8::Local<v8::Context> context,
                             v8::Local<v8::String> specifier,
                             v8::Local<v8::FixedArray> import_attributes);
  static ModuleCacheKey From(v8::Local<v8::Context> context, v8::Local<v8::ModuleRequest> v8_request);

  struct Hash {
    std::size_t operator()(const ModuleCacheKey& request) const { return request.hash; }
  };

  bool operator==(const ModuleCacheKey& other) const {
    return specifier == other.specifier && import_attributes == other.import_attributes;
  }

 private:
  ModuleCacheKey(std::string specifier, ImportAttributeVector import_attributes, std::size_t hash)
      : specifier(specifier), import_attributes(import_attributes), hash(hash) {}
};

class ModuleWrap : public BaseObject {
  using ResolveCache = std::unordered_map<ModuleCacheKey, uint32_t, ModuleCacheKey::Hash>;

 public:
  enum Status { kUninstantiated, kInstantiating, kInstantiated, kEvaluating, kEvaluated, kErrored };

  enum HostDefinedOptions : int {
    kID = 8,
    kLength = 9,
  };

  enum InternalFields {
    kModuleSlot = BaseObject::kInternalFieldCount,
    kURLSlot,
    kModuleSourceObjectSlot,
    kSyntheticEvaluationStepsSlot,
    kContextObjectSlot,
    kLinkedRequestsSlot,
    kInternalFieldCount,
  };

  static void CreatePerIsolateProperties(IsolateData* isolate_data, v8::Local<v8::ObjectTemplate> target);
  static void CreatePerContextProperties(v8::Local<v8::Object> target, v8::Local<v8::Context> context);
  static void HostInitializeImportMetaObjectCallback(v8::Local<v8::Context> context,
                                                     v8::Local<v8::Module> module,
                                                     v8::Local<v8::Object> meta);

  v8::Local<v8::Context> context() const;
  bool HasAsyncGraph();

  bool IsLinked() const { return linked_; }

  ModuleWrap* GetLinkedRequest(uint32_t index);

  static v8::Local<v8::PrimitiveArray> GetHostDefinedOptions(v8::Isolate* isolate, v8::Local<v8::Symbol> symbol);

  static v8::MaybeLocal<v8::Module> CompileSourceTextModule(Realm* realm,
                                                            v8::Local<v8::String> source_text,
                                                            v8::Local<v8::String> url,
                                                            int line_offset,
                                                            int column_offset,
                                                            v8::Local<v8::PrimitiveArray> host_defined_options);

  static void CreateRequiredModuleFacade(const v8::FunctionCallbackInfo<v8::Value>& args);

 private:
  ModuleWrap(Realm* realm,
             v8::Local<v8::Object> object,
             v8::Local<v8::Module> module,
             v8::Local<v8::String> url,
             v8::Local<v8::Object> context_object,
             v8::Local<v8::Value> synthetic_evaluation_step);
  ~ModuleWrap();

  // new ModuleWrap(url, exportNames, evaluationCallback)
  // new ModuleWrap(url, source, lineOffset, columnOffset);
  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void GetModuleRequests(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void SetModuleSourceObject(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void GetModuleSourceObject(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void Link(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Instantiate(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Evaluate(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void EvaluateSync(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void GetNamespace(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void GetStatus(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void GetError(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void HasAsyncGraph(v8::Local<v8::Name> property, const v8::PropertyCallbackInfo<v8::Value>& args);

  static void SetImportModuleDynamicallyCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void SetInitializeImportMetaObjectCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::MaybeLocal<v8::Value> SyntheticModuleEvaluationStepsCallback(v8::Local<v8::Context> context,
                                                                          v8::Local<v8::Module> module);
  static void SetSyntheticExport(const v8::FunctionCallbackInfo<v8::Value>& args);

  static v8::MaybeLocal<v8::Module> ResolveModuleCallback(v8::Local<v8::Context> context,
                                                          v8::Local<v8::String> specifier,
                                                          v8::Local<v8::FixedArray> import_attributes,
                                                          v8::Local<v8::Module> referrer);
  static ModuleWrap* GetFromModule(Environment* env, v8::Local<v8::Module> module);

  static v8::Maybe<ModuleWrap*> ResolveModule(v8::Local<v8::Context> context,
                                              v8::Local<v8::String> specifier,
                                              v8::Local<v8::FixedArray> import_attributes,
                                              v8::Local<v8::Module> referrer);

 private:
  v8::Global<v8::Module> module_;
  ResolveCache resolve_cache_;
  bool synthetic_ = false;
  bool linked_ = false;
  std::optional<bool> has_async_graph_ = std::nullopt;
  int module_hash_;

  // For synthetic modules: stores the exports object
  v8::Global<v8::Object> synthetic_exports_;
};

}  // namespace nyx
