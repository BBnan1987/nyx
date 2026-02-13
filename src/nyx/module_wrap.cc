#include "nyx/module_wrap.h"

#include "nyx/env.h"
#include "nyx/errors.h"
#include "nyx/nyx_binding.h"
#include "nyx/realm.h"

namespace nyx {

using v8::Array;
using v8::Boolean;
using v8::Context;
using v8::Data;
using v8::EscapableHandleScope;
using v8::Exception;
using v8::FixedArray;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::HandleScope;
using v8::Int32;
using v8::Integer;
using v8::Isolate;
using v8::Just;
using v8::JustVoid;
using v8::Local;
using v8::LocalVector;
using v8::Maybe;
using v8::MaybeLocal;
using v8::MemorySpan;
using v8::Message;
using v8::Module;
using v8::ModuleRequest;
using v8::Name;
using v8::Nothing;
using v8::Object;
using v8::ObjectTemplate;
using v8::PrimitiveArray;
using v8::Promise;
using v8::PromiseRejectEvent;
using v8::PropertyAttribute;
using v8::ScriptCompiler;
using v8::ScriptOrigin;
using v8::String;
using v8::Symbol;
using v8::Value;

std::string ModuleCacheKey::ToString() const {
  std::string result = "ModuleCacheKey(\"" + specifier + "\"";
  if (!import_attributes.empty()) {
    result += ", {";
    bool first = true;
    for (const auto& attr : import_attributes) {
      if (first) {
        first = false;
      } else {
        result += ", ";
      }
      result += attr.first + ": " + attr.second;
    }
    result += "}";
  }
  result += ")";
  return result;
}

template <int elements_per_attribute>
inline ModuleCacheKey ModuleCacheKey::From(Local<Context> context,
                                           Local<String> specifier,
                                           Local<FixedArray> import_attributes) {
  CHECK_EQ(import_attributes->Length() % elements_per_attribute, 0);
  Isolate* isolate = context->GetIsolate();
  std::size_t h1 = specifier->GetIdentityHash();
  size_t num_attributes = import_attributes->Length() / elements_per_attribute;
  ImportAttributeVector attributes;
  attributes.reserve(num_attributes);

  std::size_t h2 = 0;

  for (int i = 0; i < import_attributes->Length(); i += elements_per_attribute) {
    Local<String> v8_key = import_attributes->Get(context, i).As<String>();
    Local<String> v8_value = import_attributes->Get(context, i + 1).As<String>();
    Utf8Value key_utf8(isolate, v8_key);
    Utf8Value value_utf8(isolate, v8_value);

    attributes.emplace_back(key_utf8.ToString(), value_utf8.ToString());
    h2 ^= v8_key->GetIdentityHash();
    h2 ^= v8_value->GetIdentityHash();
  }

  std::size_t hash = h1 ^ (h2 << 1);

  Utf8Value utf8_specifier(isolate, specifier);
  return ModuleCacheKey{utf8_specifier.ToString(), attributes, hash};
}

ModuleCacheKey ModuleCacheKey::From(Local<Context> context, Local<ModuleRequest> v8_request) {
  return From(context, v8_request->GetSpecifier(), v8_request->GetImportAttributes());
}

static void ThrowIfPromiseRejected(const FunctionCallbackInfo<Value>& args);

void ModuleWrap::CreatePerIsolateProperties(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();

  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, New);
  tmpl->InstanceTemplate()->SetInternalFieldCount(kInternalFieldCount);
  tmpl->SetClassName(OneByteString(isolate, "ModuleWrap"));

  SetProtoMethod(isolate, tmpl, "link", Link);
  SetProtoMethod(isolate, tmpl, "getModuleRequests", GetModuleRequests);
  SetProtoMethod(isolate, tmpl, "instantiate", Instantiate);
  SetProtoMethod(isolate, tmpl, "evaluate", Evaluate);
  SetProtoMethod(isolate, tmpl, "evaluateSync", EvaluateSync);
  SetProtoMethod(isolate, tmpl, "setExport", SetSyntheticExport);
  SetProtoMethod(isolate, tmpl, "setModuleSourceObject", SetModuleSourceObject);
  SetProtoMethod(isolate, tmpl, "getModuleSourceObject", GetModuleSourceObject);
  SetProtoMethod(isolate, tmpl, "getNamespace", GetNamespace);
  SetProtoMethod(isolate, tmpl, "getStatus", GetStatus);
  SetProtoMethod(isolate, tmpl, "getError", GetError);

  tmpl->InstanceTemplate()->SetLazyDataProperty(
      FixedOneByteString(isolate, "hasAsyncGraph"), HasAsyncGraph, Local<Value>(), PropertyAttribute::DontEnum);

  target->Set(OneByteString(isolate, "ModuleWrap"), tmpl);
  isolate_data->set_module_wrap_constructor_template(tmpl);

  SetMethod(isolate, target, "setImportModuleDynamicallyCallback", SetImportModuleDynamicallyCallback);
  SetMethod(isolate, target, "setInitializeImportMetaObjectCallback", SetInitializeImportMetaObjectCallback);
  SetMethod(isolate, target, "createRequiredModuleFacade", CreateRequiredModuleFacade);
  SetMethod(isolate, target, "throwIfPromiseRejected", ThrowIfPromiseRejected);
}

void ModuleWrap::CreatePerContextProperties(Local<Object> target, Local<Context> context) {
  Realm* realm = Realm::GetCurrent(context);
  Isolate* isolate = realm->isolate();
#define V(enum_type, name)                                                                                             \
  target->Set(context, FixedOneByteString(isolate, #name), Integer::New(isolate, enum_type::name)).FromJust()
  V(Module::Status, kUninstantiated);
  V(Module::Status, kInstantiating);
  V(Module::Status, kInstantiated);
  V(Module::Status, kEvaluating);
  V(Module::Status, kEvaluated);
  V(Module::Status, kErrored);
#undef V
}

void ModuleWrap::HostInitializeImportMetaObjectCallback(Local<Context> context,
                                                        Local<Module> module,
                                                        Local<Object> meta) {
  Environment* env = Environment::GetCurrent(context);
  if (env == nullptr) {
    return;
  }
  ModuleWrap* module_wrap = GetFromModule(env, module);
  if (module_wrap == nullptr) {
    return;
  }
  Realm* realm = Realm::GetCurrent(context);
  if (realm == nullptr) {
    realm = reinterpret_cast<Realm*>(env->principal_realm());
  }

  Local<Object> wrap = module_wrap->object();
  Local<Function> callback = realm->host_initialize_import_meta_object_callback();
  Local<Value> id;
  if (!wrap->GetPrivate(context, env->host_defined_option_symbol()).ToLocal(&id)) {
    return;
  }
  Local<Value> args[] = {id, meta, wrap};
  TryCatchScope try_catch(realm->isolate());
  callback->Call(context, Undefined(realm->isolate()), arraysize(args), args);
  if (try_catch.HasCaught() && !try_catch.HasTerminated()) {
    try_catch.ReThrow();
  }
}

Local<Context> ModuleWrap::context() const {
  Local<Value> obj = object()->GetInternalField(kContextObjectSlot).As<Value>();
  CHECK(obj->IsObject());
  return obj.As<Object>()->GetCreationContextChecked();
}

bool ModuleWrap::HasAsyncGraph() {
  if (!has_async_graph_.has_value()) {
    Isolate* isolate = env()->isolate();
    HandleScope scope(isolate);
    has_async_graph_ = module_.Get(isolate)->IsGraphAsync();
  }
  return has_async_graph_.value();
}

ModuleWrap* ModuleWrap::GetLinkedRequest(uint32_t index) {
  Isolate* isolate = env()->isolate();
  EscapableHandleScope scope(isolate);
  Local<Data> linked_requests_data = object()->GetInternalField(kLinkedRequestsSlot);
  Local<Array> requests = linked_requests_data.As<Value>().As<Array>();

  CHECK_LT(index, requests->Length());

  Local<Value> module_value;
  if (!requests->Get(context(), index).ToLocal(&module_value)) {
    return nullptr;
  }
  CHECK(module_value->IsObject());
  Local<Object> module_object = module_value.As<Object>();

  ModuleWrap* module_wrap;
  ASSIGN_OR_RETURN_UNWRAP(&module_wrap, module_object, nullptr);
  return module_wrap;
}

Local<PrimitiveArray> ModuleWrap::GetHostDefinedOptions(Isolate* isolate, Local<Symbol> symbol) {
  Local<PrimitiveArray> host_defined_options = PrimitiveArray::New(isolate, HostDefinedOptions::kLength);
  host_defined_options->Set(isolate, HostDefinedOptions::kID, symbol);
  return host_defined_options;
}

MaybeLocal<Module> ModuleWrap::CompileSourceTextModule(Realm* realm,
                                                       Local<String> source_text,
                                                       Local<String> url,
                                                       int line_offset,
                                                       int column_offset,
                                                       Local<PrimitiveArray> host_defined_options) {
  Isolate* isolate = realm->isolate();
  EscapableHandleScope scope(isolate);

  ScriptOrigin origin(
      url, line_offset, column_offset, true, -1, Local<Value>(), false, false, true, host_defined_options);
  ScriptCompiler::Source source(source_text, origin);

  Local<Module> module;
  if (!ScriptCompiler::CompileModule(isolate, &source, ScriptCompiler::kNoCompileOptions).ToLocal(&module)) {
    return scope.EscapeMaybe(MaybeLocal<Module>());
  }

  return scope.Escape(module);
}

static MaybeLocal<Module> LinkRequireFacadeWithOriginal(Local<Context> context,
                                                        Local<String> specifier,
                                                        Local<FixedArray> import_attributes,
                                                        Local<Module> referrer) {
  Environment* env = Environment::GetCurrent(context);
  Isolate* isolate = context->GetIsolate();
  CHECK(specifier->Equals(context, env->original_string()).ToChecked());
  CHECK(!env->temporary_required_module_facade_original.IsEmpty());
  return env->temporary_required_module_facade_original.Get(isolate);
}

void ModuleWrap::CreateRequiredModuleFacade(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  CHECK(args[0]->IsObject());
  Local<Object> wrap = args[0].As<Object>();
  ModuleWrap* original;
  ASSIGN_OR_RETURN_UNWRAP(&original, wrap);

  ScriptOrigin origin(env->required_module_facade_url_string(), 0, 0, true, -1, Local<Value>(), false, false, true);
  ScriptCompiler::Source source(env->required_module_facade_source_string(), origin);

  Local<Module> facade;
  if (!ScriptCompiler::CompileModule(isolate, &source).ToLocal(&facade)) {
    return;
  }
  CHECK(env->temporary_required_module_facade_original.IsEmpty());
  env->temporary_required_module_facade_original.Reset(isolate, original->module_.Get(isolate));
  CHECK(facade->InstantiateModule(context, LinkRequireFacadeWithOriginal).IsJust());
  env->temporary_required_module_facade_original.Reset();

  Local<Value> evaluated;
  if (!facade->Evaluate(context).ToLocal(&evaluated)) {
    return;
  }
  CHECK(evaluated->IsPromise());
  CHECK_EQ(evaluated.As<Promise>()->State(), Promise::PromiseState::kFulfilled);
  args.GetReturnValue().Set(facade->GetModuleNamespace());
}

ModuleWrap::ModuleWrap(Realm* realm,
                       Local<Object> object,
                       Local<Module> module,
                       Local<String> url,
                       Local<Object> context_object,
                       Local<Value> synthetic_evaluation_step)
    : BaseObject(realm, object), module_(realm->isolate(), module), module_hash_(module->GetIdentityHash()) {
  env()->RegisterModule(module->GetIdentityHash(), this);

  object->SetInternalField(kModuleSlot, module);
  object->SetInternalField(kURLSlot, url);
  object->SetInternalField(kModuleSourceObjectSlot, Undefined(realm->isolate()));
  object->SetInternalField(kSyntheticEvaluationStepsSlot, synthetic_evaluation_step);
  object->SetInternalField(kContextObjectSlot, context_object);
  object->SetInternalField(kLinkedRequestsSlot, Undefined(realm->isolate()));

  if (!synthetic_evaluation_step->IsUndefined()) {
    synthetic_ = true;
    linked_ = true;
  }
  MakeWeak();
  module_.SetWeak();

  HandleScope scope(realm->isolate());
  Local<Context> context = realm->context();
  Local<FixedArray> requests = module->GetModuleRequests();
  for (int i = 0; i < requests->Length(); ++i) {
    ModuleCacheKey module_cache_key = ModuleCacheKey::From(context, requests->Get(context, i).As<ModuleRequest>());
    resolve_cache_[module_cache_key] = i;
  }
}

ModuleWrap::~ModuleWrap() {
  if (!module_.IsEmpty()) {
    env()->UnregisterModule(module_hash_);
  }
  module_.Reset();
  synthetic_exports_.Reset();
}

void ModuleWrap::New(const FunctionCallbackInfo<Value>& args) {
  CHECK(args.IsConstructCall());
  CHECK_GE(args.Length(), 2);

  Realm* realm = Realm::GetCurrent(args);
  Isolate* isolate = realm->isolate();

  Local<Object> that = args.This();

  CHECK(args[0]->IsString());
  Local<String> url = args[0].As<String>();

  Local<Context> context = that->GetCreationContextChecked();

  int line_offset = 0;
  int column_offset = 0;

  bool synthetic = args[1]->IsArray();
  Local<PrimitiveArray> host_defined_options = PrimitiveArray::New(isolate, HostDefinedOptions::kLength);
  Local<Symbol> id_symbol;
  if (synthetic) {
    // new ModuleWrap(url, exportNames, evaluationCallback)
    CHECK(args[2]->IsFunction());
  } else {
    // new ModuleWrap(url, source, lineOffset, columnOffset, idSymbol);
    CHECK(args[1]->IsString());
    CHECK(args[2]->IsNumber());
    line_offset = args[2].As<Int32>()->Value();
    CHECK(args[3]->IsNumber());
    column_offset = args[3].As<Int32>()->Value();
    if (args[4]->IsSymbol()) {
      id_symbol = args[4].As<Symbol>();
    } else {
      id_symbol = Symbol::New(isolate, url);
    }
    host_defined_options = GetHostDefinedOptions(isolate, id_symbol);

    if (that->SetPrivate(context, realm->isolate_data()->host_defined_option_symbol(), id_symbol).IsNothing()) {
      return;
    }
  }

  TryCatchScope try_catch(realm->isolate());

  Local<Module> module;
  {
    Context::Scope context_scope(context);
    if (synthetic) {
      Local<Array> export_names_arr = args[1].As<Array>();

      uint32_t len = export_names_arr->Length();
      LocalVector<String> export_names(realm->isolate(), len);
      for (uint32_t i = 0; i < len; ++i) {
        Local<Value> export_name_val;
        if (!export_names_arr->Get(context, i).ToLocal(&export_name_val)) {
          return;
        }
        CHECK(export_name_val->IsString());
        export_names[i] = export_name_val.As<String>();
      }

      const MemorySpan<const Local<String>> span(export_names.begin(), export_names.size());
      module = Module::CreateSyntheticModule(isolate, url, span, SyntheticModuleEvaluationStepsCallback);
    } else {
      Local<String> source_text = args[1].As<String>();

      if (!CompileSourceTextModule(realm, source_text, url, line_offset, column_offset, host_defined_options)
               .ToLocal(&module)) {
        if (try_catch.HasCaught() && !try_catch.HasTerminated()) {
          CHECK(!try_catch.Message().IsEmpty());
          CHECK(!try_catch.Exception().IsEmpty());
          try_catch.ReThrow();
        }
        return;
      }

      if (that->Set(context, realm->env()->source_url_string(), module->GetUnboundModuleScript()->GetSourceURL())
              .IsNothing()) {
        return;
      }

      if (that->Set(context,
                    realm->env()->source_map_url_string(),
                    module->GetUnboundModuleScript()->GetSourceMappingURL())
              .IsNothing()) {
        return;
      }
    }
  }

  if (that->Set(context, realm->isolate_data()->synthetic_string(), Boolean::New(isolate, synthetic)).IsNothing()) {
    return;
  }

  if (that->Set(context, realm->isolate_data()->url_string(), url).IsNothing()) {
    return;
  }

  if (that->SetPrivate(context, realm->isolate_data()->source_map_data_private_symbol(), Undefined(isolate))
          .IsNothing()) {
    return;
  }

  Local<Object> context_object = context->GetExtrasBindingObject();
  Local<Value> synthetic_evaluation_step = synthetic ? args[2] : Undefined(realm->isolate()).As<Value>();

  ModuleWrap* obj = new ModuleWrap(realm, that, module, url, context_object, synthetic_evaluation_step);
  args.GetReturnValue().Set(that);
}

static Local<Object> CreateImportAttributesContainer(Realm* realm,
                                                     Isolate* isolate,
                                                     Local<FixedArray> raw_attributes,
                                                     const int elements_per_attribute) {
  CHECK_EQ(raw_attributes->Length() % elements_per_attribute, 0);
  size_t num_attributes = raw_attributes->Length() / elements_per_attribute;
  LocalVector<Name> names(isolate, num_attributes);
  LocalVector<Value> values(isolate, num_attributes);

  for (int i = 0; i < raw_attributes->Length(); i += elements_per_attribute) {
    Local<Data> key = raw_attributes->Get(realm->context(), i);
    Local<Data> value = raw_attributes->Get(realm->context(), i + 1);

    int idx = i / elements_per_attribute;
    names[idx] = key.As<String>();
    values[idx] = value.As<String>();
  }

  Local<Object> attributes = Object::New(isolate, Null(isolate), names.data(), values.data(), num_attributes);
  attributes->SetIntegrityLevel(realm->context(), v8::IntegrityLevel::kFrozen).Check();
  return attributes;
}

static Local<Array> CreateModuleRequestsContainer(Realm* realm, Isolate* isolate, Local<FixedArray> raw_requests) {
  EscapableHandleScope scope(isolate);
  Local<Context> context = realm->context();
  LocalVector<Value> requests(isolate, raw_requests->Length());

  for (int i = 0; i < raw_requests->Length(); ++i) {
    Local<ModuleRequest> module_request = raw_requests->Get(realm->context(), i).As<ModuleRequest>();

    Local<String> specifier = module_request->GetSpecifier();

    Local<FixedArray> raw_attributes = module_request->GetImportAttributes();
    Local<Object> attributes = CreateImportAttributesContainer(realm, isolate, raw_attributes, 3);

    Local<Name> names[] = {
        realm->isolate_data()->specifier_string(),
        realm->isolate_data()->attributes_string(),
    };
    Local<Value> values[] = {
        specifier,
        attributes,
    };

    Local<Object> request = Object::New(isolate, Null(isolate), names, values, arraysize(names));
    request->SetIntegrityLevel(context, v8::IntegrityLevel::kFrozen).Check();
    requests[i] = request;
  }

  return scope.Escape(Array::New(isolate, requests.data(), requests.size()));
}

void ModuleWrap::GetModuleRequests(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Realm* realm = Realm::GetCurrent(args);
  Isolate* isolate = args.GetIsolate();
  Local<Object> that = args.This();

  ModuleWrap* obj;
  ASSIGN_OR_RETURN_UNWRAP(&obj, that);

  Local<Module> module = obj->module_.Get(isolate);
  args.GetReturnValue().Set(CreateModuleRequestsContainer(realm, isolate, module->GetModuleRequests()));
}

void ModuleWrap::SetModuleSourceObject(const v8::FunctionCallbackInfo<v8::Value>& args) {
  ModuleWrap* obj;
  ASSIGN_OR_RETURN_UNWRAP(&obj, args.This());

  CHECK_EQ(args.Length(), 1);
  CHECK(args[0]->IsObject());
  CHECK(obj->object()->GetInternalField(kModuleSourceObjectSlot).As<Value>()->IsUndefined());
  obj->object()->SetInternalField(kModuleSourceObjectSlot, args[0]);
}

void ModuleWrap::GetModuleSourceObject(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  ModuleWrap* obj;
  ASSIGN_OR_RETURN_UNWRAP(&obj, args.This());

  CHECK_EQ(args.Length(), 0);
  Local<Value> module_source_object = obj->object()->GetInternalField(kModuleSourceObjectSlot).As<Value>();

  if (module_source_object->IsUndefined()) {
    Utf8Value url(isolate, obj->object()->GetInternalField(kURLSlot).As<String>());
    THROW_ERR_SOURCE_PHASE_NOT_DEFINED(isolate, *url);
    return;
  }

  args.GetReturnValue().Set(module_source_object);
}

void ModuleWrap::Link(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Realm* realm = Realm::GetCurrent(args);
  Local<Context> context = realm->context();

  ModuleWrap* dependent;
  ASSIGN_OR_RETURN_UNWRAP(&dependent, args.This());

  CHECK_EQ(args.Length(), 1);
  CHECK(args[0]->IsArray());

  Local<FixedArray> requests = dependent->module_.Get(isolate)->GetModuleRequests();
  Local<Array> modules = args[0].As<Array>();
  CHECK_EQ(modules->Length(), static_cast<uint32_t>(requests->Length()));

  for (int i = 0; i < requests->Length(); ++i) {
    ModuleCacheKey module_cache_key = ModuleCacheKey::From(context, requests->Get(context, i).As<ModuleRequest>());

    Local<Value> module_i;
    Local<Value> module_cache_i;
    uint32_t coalesced_index = dependent->resolve_cache_[module_cache_key];
    if (!modules->Get(context, i).ToLocal(&module_i) ||
        !modules->Get(context, coalesced_index).ToLocal(&module_cache_i) || !module_i->StrictEquals(module_cache_i)) {
      THROW_ERR_MODULE_LINK_MISMATCH(isolate, module_cache_key.ToString());
      return;
    }
  }

  args.This()->SetInternalField(kLinkedRequestsSlot, modules);
  dependent->linked_ = true;
}

void ModuleWrap::Instantiate(const FunctionCallbackInfo<Value>& args) {
  Realm* realm = Realm::GetCurrent(args);
  Isolate* isolate = args.GetIsolate();
  ModuleWrap* obj;
  ASSIGN_OR_RETURN_UNWRAP(&obj, args.This());
  Local<Context> context = obj->context();
  Local<Module> module = obj->module_.Get(isolate);
  Environment* env = realm->env();

  if (!obj->IsLinked()) {
    THROW_ERR_VM_MODULE_LINK_FAILURE(isolate, "");
    return;
  }

  {
    TryCatchScope try_catch(env->isolate());
    module->InstantiateModule(context, ResolveModuleCallback);
    if (try_catch.HasCaught() && !try_catch.HasTerminated()) {
      CHECK(!try_catch.Message().IsEmpty());
      CHECK(!try_catch.Exception().IsEmpty());
      try_catch.ReThrow();
      return;
    }
  }
}

void ModuleWrap::Evaluate(const FunctionCallbackInfo<Value>& args) {
  Realm* realm = Realm::GetCurrent(args);
  Isolate* isolate = realm->isolate();
  ModuleWrap* obj;
  ASSIGN_OR_RETURN_UNWRAP(&obj, args.This());
  Local<Context> context = obj->context();
  Local<Module> module = obj->module_.Get(isolate);

  CHECK_EQ(args.Length(), 0);

  TryCatchScope try_catch(isolate);
  MaybeLocal<Value> result = module->Evaluate(context);
  if (result.IsEmpty()) {
    CHECK(try_catch.HasCaught());
  }
  if (try_catch.HasCaught()) {
    if (!try_catch.HasTerminated()) {
      try_catch.ReThrow();
    }
    return;
  }

  Local<Value> res;
  if (result.ToLocal(&res)) {
    args.GetReturnValue().Set(res);
  }
}

static Maybe<void> ThrowIfPromiseRejected(Realm* realm, Local<Promise> promise) {
  Isolate* isolate = realm->isolate();
  Local<Context> context = realm->context();
  if (promise->State() != Promise::PromiseState::kRejected) {
    return JustVoid();
  }

  Local<Value> exception = promise->Result();
  Local<Message> message = Exception::CreateMessage(isolate, exception);
  // AppendExceptionLine(env, exception, message, ErrorHandlingMode::MODULE_ERROR);
  isolate->ThrowException(exception);
  return Nothing<void>();
}

void ThrowIfPromiseRejected(const FunctionCallbackInfo<Value>& args) {
  if (!args[0]->IsPromise()) {
    return;
  }
  ThrowIfPromiseRejected(Realm::GetCurrent(args), args[0].As<Promise>());
}

void ModuleWrap::EvaluateSync(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Realm* realm = Realm::GetCurrent(args);
  Isolate* isolate = args.GetIsolate();
  ModuleWrap* obj;
  ASSIGN_OR_RETURN_UNWRAP(&obj, args.This());
  Local<Context> context = obj->context();
  Local<Module> module = obj->module_.Get(isolate);
  Environment* env = realm->env();

  Local<Value> result;
  {
    TryCatchScope try_catch(isolate);
    if (!module->Evaluate(context).ToLocal(&result)) {
      if (try_catch.HasCaught()) {
        if (!try_catch.HasTerminated()) {
          try_catch.ReThrow();
        }
        return;
      }
    }
  }

  CHECK(result->IsPromise());
  Local<Promise> promise = result.As<Promise>();
  if (ThrowIfPromiseRejected(realm, promise).IsNothing()) {
    return;
  }

  if (obj->HasAsyncGraph()) {
    auto stalled_messages = std::get<1>(module->GetStalledTopLevelAwaitMessages(isolate));
    if (stalled_messages.size() != 0) {
      for (auto& message : stalled_messages) {
        fprintf(stderr, "Error: unexpected top-level await at ?");
      }
    }
    THROW_ERR_REQUIRE_ASYNC_MODULE(isolate, "");
    return;
  }

  CHECK_EQ(promise->State(), Promise::PromiseState::kFulfilled);
  args.GetReturnValue().Set(module->GetModuleNamespace());
}

void ModuleWrap::GetNamespace(const FunctionCallbackInfo<Value>& args) {
  Realm* realm = Realm::GetCurrent(args);
  Isolate* isolate = args.GetIsolate();
  ModuleWrap* obj;
  ASSIGN_OR_RETURN_UNWRAP(&obj, args.This());

  Local<Module> module = obj->module_.Get(isolate);
  if (module->GetStatus() < Module::kInstantiated) {
    return THROW_ERR_MODULE_NOT_INSTANTIATED(isolate, "");
  }

  Local<Value> result = module->GetModuleNamespace();
  args.GetReturnValue().Set(result);
}

void ModuleWrap::GetStatus(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  ModuleWrap* obj;
  ASSIGN_OR_RETURN_UNWRAP(&obj, args.This());

  Local<Module> module = obj->module_.Get(isolate);
  args.GetReturnValue().Set(module->GetStatus());
}

void ModuleWrap::GetError(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  ModuleWrap* obj;
  ASSIGN_OR_RETURN_UNWRAP(&obj, args.This());

  Local<Module> module = obj->module_.Get(isolate);
  args.GetReturnValue().Set(module->GetException());
}

void ModuleWrap::HasAsyncGraph(v8::Local<v8::Name> property, const v8::PropertyCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  ModuleWrap* obj;
  ASSIGN_OR_RETURN_UNWRAP(&obj, args.This());

  Local<Module> module = obj->module_.Get(isolate);
  if (module->GetStatus() < Module::kInstantiated) {
    return THROW_ERR_MODULE_NOT_INSTANTIATED(isolate, "");
  }

  args.GetReturnValue().Set(obj->HasAsyncGraph());
}

static MaybeLocal<Promise> ImportModuleDynamicallyCallback(Local<Context> context,
                                                           Local<Data> host_defined_options,
                                                           Local<Value> resource_name,
                                                           Local<String> specifier,
                                                           Local<FixedArray> import_attributes) {
  Isolate* isolate = context->GetIsolate();

  // TODO: Implement dynamic import()

  isolate->ThrowException(OneByteString(isolate, "Dynamic import() not yet implemented"));
  return MaybeLocal<Promise>();
}

void ModuleWrap::SetImportModuleDynamicallyCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Realm* realm = Realm::GetCurrent(args);
  HandleScope handle_scope(isolate);

  CHECK_EQ(args.Length(), 1);
  CHECK(args[0]->IsFunction());
  Local<Function> import_callback = args[0].As<Function>();
  realm->set_host_import_module_dynamically_callback(import_callback);
  isolate->SetHostImportModuleDynamicallyCallback(ImportModuleDynamicallyCallback);
}

void ModuleWrap::SetInitializeImportMetaObjectCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Realm* realm = Realm::GetCurrent(args);
  Isolate* isolate = realm->isolate();

  CHECK_EQ(args.Length(), 1);
  CHECK(args[0]->IsFunction());
  Local<Function> import_meta_callback = args[0].As<Function>();
  realm->set_host_initialize_import_meta_object_callback(import_meta_callback);
  isolate->SetHostInitializeImportMetaObjectCallback(HostInitializeImportMetaObjectCallback);
}

MaybeLocal<Value> ModuleWrap::SyntheticModuleEvaluationStepsCallback(Local<Context> context, Local<Module> module) {
  Environment* env = Environment::GetCurrent(context);
  Isolate* isolate = env->isolate();
  ModuleWrap* wrap = GetFromModule(env, module);

  TryCatchScope try_catch(env->isolate());
  Local<Function> synthetic_evaluation_steps =
      wrap->object()->GetInternalField(kSyntheticEvaluationStepsSlot).As<Value>().As<Function>();
  wrap->object()->SetInternalField(kSyntheticEvaluationStepsSlot, Undefined(isolate));
  MaybeLocal<Value> ret = synthetic_evaluation_steps->Call(context, wrap->object(), 0, nullptr);
  if (ret.IsEmpty()) {
    CHECK(try_catch.HasCaught());
  }
  if (try_catch.HasCaught() && !try_catch.HasTerminated()) {
    CHECK(!try_catch.Message().IsEmpty());
    CHECK(!try_catch.Exception().IsEmpty());
    try_catch.ReThrow();
    return MaybeLocal<Value>();
  }

  Local<Promise::Resolver> resolver;
  if (!Promise::Resolver::New(context).ToLocal(&resolver)) {
    return MaybeLocal<Value>();
  }

  if (resolver->Resolve(context, Undefined(isolate)).IsNothing()) {
    return MaybeLocal<Value>();
  }
  return resolver->GetPromise();
}

void ModuleWrap::SetSyntheticExport(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Object> that = args.This();

  ModuleWrap* obj = Unwrap<ModuleWrap>(that);

  CHECK(obj->synthetic_);
  CHECK_EQ(args.Length(), 2);

  CHECK(args[0]->IsString());
  Local<String> export_name = args[0].As<String>();
  Local<Value> export_value = args[1];
  Local<Module> module = obj->module_.Get(isolate);
  module->SetSyntheticModuleExport(isolate, export_name, export_value);
}

MaybeLocal<Module> ModuleWrap::ResolveModuleCallback(Local<Context> context,
                                                     Local<String> specifier,
                                                     Local<FixedArray> import_attributes,
                                                     Local<Module> referrer) {
  ModuleWrap* resolved_module;
  if (!ResolveModule(context, specifier, import_attributes, referrer).To(&resolved_module)) {
    return {};
  }
  return resolved_module->module_.Get(context->GetIsolate());
}

ModuleWrap* ModuleWrap::GetFromModule(Environment* env, Local<Module> module) {
  return env->GetModuleWrap(module);
}

v8::Maybe<ModuleWrap*> ModuleWrap::ResolveModule(v8::Local<v8::Context> context,
                                                 v8::Local<v8::String> specifier,
                                                 v8::Local<v8::FixedArray> import_attributes,
                                                 v8::Local<v8::Module> referrer) {
  Isolate* isolate = context->GetIsolate();
  Environment* env = Environment::GetCurrent(context);
  if (env == nullptr) {
    THROW_ERR_EXECUTION_ENVIRONMENT_NOT_AVAILABLE(isolate);
    return Nothing<ModuleWrap*>();
  }

  ModuleCacheKey cache_key = ModuleCacheKey::From(context, specifier, import_attributes);

  ModuleWrap* dependent = ModuleWrap::GetFromModule(env, referrer);
  if (dependent == nullptr) {
    THROW_ERR_VM_MODULE_LINK_FAILURE(isolate, cache_key.specifier);
    return Nothing<ModuleWrap*>();
  }
  if (!dependent->IsLinked()) {
    THROW_ERR_VM_MODULE_LINK_FAILURE(isolate, cache_key.specifier);
    return Nothing<ModuleWrap*>();
  }

  auto it = dependent->resolve_cache_.find(cache_key);
  if (it == dependent->resolve_cache_.end()) {
    THROW_ERR_VM_MODULE_LINK_FAILURE(isolate, cache_key.specifier);
    return Nothing<ModuleWrap*>();
  }

  ModuleWrap* module_wrap = dependent->GetLinkedRequest(it->second);
  CHECK_NOT_NULL(module_wrap);
  return Just(module_wrap);
}

NYX_BINDING_PER_ISOLATE_INIT(module_wrap, ModuleWrap::CreatePerIsolateProperties)
NYX_BINDING_CONTEXT_AWARE(module_wrap, ModuleWrap::CreatePerContextProperties)

}  // namespace nyx
