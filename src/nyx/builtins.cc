#include "nyx/builtins.h"

#include "nyx/env.h"
#include "nyx/errors.h"
#include "nyx/extension.h"
#include "nyx/isolate_data.h"
#include "nyx/realm.h"
#include "nyx/util.h"

#include <fstream>
#include <sstream>

namespace nyx {

using v8::Context;
using v8::EscapableHandleScope;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::HandleScope;
using v8::IntegrityLevel;
using v8::Isolate;
using v8::Local;
using v8::LocalVector;
using v8::MaybeLocal;
using v8::Name;
using v8::None;
using v8::Object;
using v8::ObjectTemplate;
using v8::PropertyCallbackInfo;
using v8::ScriptCompiler;
using v8::ScriptOrigin;
using v8::SideEffectType;
using v8::String;
using v8::Value;

BuiltinLoader::BuiltinLoader() {
  LoadJavaScriptSource();
  RegisterExternalBuiltins();
#ifdef NYX_DEBUG
  prefer_disk_ = true;
#endif
}

static void SetInternalLoaders(const FunctionCallbackInfo<Value>& args) {
  Realm* realm = Realm::GetCurrent(args);
  CHECK(args[0]->IsFunction());
  CHECK(args[1]->IsFunction());
  realm->set_internal_binding_loader(args[0].As<Function>());
  realm->set_builtin_module_require(args[1].As<Function>());
}

void BuiltinLoader::CreatePerIsolateProperties(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();

  target->SetNativeDataProperty(FixedOneByteString(isolate, "builtinIds"),
                                BuiltinIdsGetter,
                                nullptr,
                                Local<Value>(),
                                None,
                                SideEffectType::kHasNoSideEffect);
  SetMethod(isolate, target, "compileFunction", CompileFunction);
  SetMethod(isolate, target, "setInternalLoaders", SetInternalLoaders);
}

void BuiltinLoader::CreatePerContextProperties(Local<Object> target, Local<Context> context) {
  target->SetIntegrityLevel(context, IntegrityLevel::kFrozen).FromJust();
}

MaybeLocal<Function> BuiltinLoader::LookupAndCompile(Local<Context> context, const char* id, Realm* optional_realm) {
  Isolate* isolate = context->GetIsolate();
  LocalVector<String> parameters(isolate);
  if (strcmp(id, "internal/bootstrap/realm") == 0) {
    // Bootstrap loader gets getInternalBinding only
    parameters = {
        FixedOneByteString(isolate, "getInternalBinding"),
    };
  } else if (strncmp(id, "internal/main/", strlen("internal/main/")) == 0 ||
             strncmp(id, "internal/bootstrap/", strlen("internal/bootstrap/")) == 0) {
    parameters = {
        FixedOneByteString(isolate, "require"),
        FixedOneByteString(isolate, "internalBinding"),
    };
  } else {
    parameters = {
        FixedOneByteString(isolate, "exports"),
        FixedOneByteString(isolate, "require"),
        FixedOneByteString(isolate, "module"),
        FixedOneByteString(isolate, "internalBinding"),
    };
  }
  MaybeLocal<Function> maybe = LookupAndCompileInternal(context, id, &parameters, optional_realm);
  return maybe;
}

MaybeLocal<Function> BuiltinLoader::LookupAndCompile(Local<Context> context,
                                                     const char* id,
                                                     LocalVector<String>* parameters,
                                                     Realm* optional_realm) {
  return LookupAndCompileInternal(context, id, parameters, optional_realm);
}

MaybeLocal<Value> BuiltinLoader::CompileAndCall(
    Local<Context> context, const char* id, int argc, Local<Value> argv[], Realm* optional_realm) {
  MaybeLocal<Function> maybe_fn = LookupAndCompile(context, id, optional_realm);
  Local<Function> fn;
  if (!maybe_fn.ToLocal(&fn)) {
    return MaybeLocal<Value>();
  }
  Local<Value> undefined = Undefined(context->GetIsolate());
  return fn->Call(context, undefined, argc, argv);
}

MaybeLocal<Value> BuiltinLoader::CompileAndCall(Local<Context> context, const char* id, Realm* realm) {
  Isolate* isolate = context->GetIsolate();
  if (strcmp(id, "internal/bootstrap/realm") == 0) {
    Local<Value> get_internal_binding;
    if (!NewFunctionTemplate(isolate, GetInternalBinding)->GetFunction(context).ToLocal(&get_internal_binding)) {
      return MaybeLocal<Value>();
    }
    Local<Value> arguments[] = {
        get_internal_binding,
    };
    return CompileAndCall(context, id, static_cast<int>(arraysize(arguments)), &arguments[0], realm);
  } else if (strncmp(id, "internal/main/", strlen("internal/main/")) == 0 ||
             strncmp(id, "internal/bootstrap/", strlen("internal/bootstrap/")) == 0) {
    Local<Value> arguments[] = {
        realm->builtin_module_require(),
        realm->internal_binding_loader(),
    };
    return CompileAndCall(context, id, static_cast<int>(arraysize(arguments)), &arguments[0], realm);
  }
  UNREACHABLE();
}  // namespace nyx

std::ranges::keys_view<std::ranges::ref_view<const BuiltinSourceMap>> BuiltinLoader::GetBuiltinIds() const {
  return std::views::keys(source_);
}

MaybeLocal<String> BuiltinLoader::LoadBuiltinSource(Isolate* isolate, const char* id) const {
  const auto source_it = source_.find(id);
  if (source_it == source_.end()) {
    THROW_ERR_OPERATION_FAILED(isolate, "Cannot find native builtin");
    return MaybeLocal<String>();
  }
  return source_it->second.ToStringChecked(isolate);
}

MaybeLocal<Function> BuiltinLoader::LookupAndCompileInternal(Local<Context> context,
                                                             const char* id,
                                                             LocalVector<String>* parameters,
                                                             Realm* optional_realm) {
  Isolate* isolate = context->GetIsolate();
  EscapableHandleScope scope(isolate);

  Local<String> source;
  if (!LoadBuiltinSource(isolate, id).ToLocal(&source)) {
    return MaybeLocal<Function>();
  }

  std::string filename_s = std::string("nyx:") + id;
  Local<String> filename = OneByteString(isolate, filename_s);
  ScriptOrigin origin(filename, 0, 0, true);

  ScriptCompiler::Source script_source(source, origin);
  MaybeLocal<Function> maybe_fun =
      ScriptCompiler::CompileFunction(context, &script_source, parameters->size(), parameters->data());

  Local<Function> fun;
  if (!maybe_fun.ToLocal(&fun)) {
    return MaybeLocal<v8::Function>();
  }

  return scope.Escape(fun);
}

void BuiltinLoader::BuiltinIdsGetter(Local<Name> property, const PropertyCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  Isolate* isolate = env->isolate();

  auto ids = env->builtin_loader()->GetBuiltinIds();
  Local<Value> ret;
  if (ToV8Value(isolate->GetCurrentContext(), ids).ToLocal(&ret)) {
    args.GetReturnValue().Set(ret);
  }
}

void BuiltinLoader::CompileFunction(const FunctionCallbackInfo<Value>& args) {
  Realm* realm = Realm::GetCurrent(args);
  CHECK(args[0]->IsString());
  String::Utf8Value id_v(realm->isolate(), args[0].As<String>());
  const char* id = *id_v;
  MaybeLocal<Function> maybe = realm->env()->builtin_loader()->LookupAndCompile(realm->context(), id, realm);
  Local<Function> fn;
  if (maybe.ToLocal(&fn)) {
    args.GetReturnValue().Set(fn);
  }
}

void BuiltinLoader::SetLibPath(const std::string& path) {
  lib_path_ = path;
  if (!lib_path_.empty() && lib_path_.back() != '/' && lib_path_.back() != '\\') {
    lib_path_ += '/';
  }
}

void BuiltinLoader::ReloadFromDisk() {
  disk_sources_.clear();
}

void BuiltinLoader::RegisterExternalBuiltins() {
  // First, merge any registered BuiltinSourceMaps (from js2c --external)
  // These are zero-copy since they point to static data
  const auto& source_maps = GetExternalBuiltinSourceMaps();
  for (const BuiltinSourceMap* map : source_maps) {
    for (const auto& [id, bytes] : *map) {
      source_[id] = bytes;
    }
  }

  // Then, handle individual RegisterBuiltin calls (requires string copy)
  const auto& external_builtins = GetExternalBuiltins();
  for (const auto& builtin : external_builtins) {
    source_[builtin.id] = UnionBytes(builtin.source);
  }
}

NYX_BINDING_PER_ISOLATE_INIT(builtins, BuiltinLoader::CreatePerIsolateProperties)
NYX_BINDING_CONTEXT_AWARE(builtins, BuiltinLoader::CreatePerContextProperties)

}  // namespace nyx
