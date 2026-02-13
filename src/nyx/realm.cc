#include "nyx/realm.h"

#include "nyx/env.h"
#include "nyx/errors.h"
#include "nyx/nyx.h"

namespace nyx {

using v8::Context;
using v8::EscapableHandleScope;
using v8::FunctionCallbackInfo;
using v8::HandleScope;
using v8::Isolate;
using v8::Local;
using v8::MaybeLocal;
using v8::ObjectTemplate;
using v8::Script;
using v8::String;
using v8::Value;

Realm::Realm(Environment* env) : env_(env), isolate_(env->isolate()) {}

Realm::Realm(Environment* env, Local<Context> context) : env_(env), isolate_(env->isolate()) {
  SetContext(context);
}

Realm::~Realm() {
  context_.Reset();
}

void Realm::SetContext(Local<Context> context) {
  context->SetAlignedPointerInEmbedderData(ContextEmbedderIndex::kEnvironment, env_);
  context->SetAlignedPointerInEmbedderData(ContextEmbedderIndex::kRealm, this);
  context_.Reset(isolate_, context);
}

Realm* Realm::GetCurrent(Isolate* isolate) {
  if (!isolate->InContext()) [[unlikely]] {
    return nullptr;
  }
  return GetCurrent(isolate->GetCurrentContext());
}

Realm* Realm::GetCurrent(Local<Context> context) {
  return static_cast<Realm*>(context->GetAlignedPointerFromEmbedderData(ContextEmbedderIndex::kRealm));
}

Realm* Realm::GetCurrent(const FunctionCallbackInfo<Value>& args) {
  return GetCurrent(args.GetIsolate());
}

void Realm::CreateProperties() {
  // Empty for now but should contain setting up Realm for executing internal/bootstrap/realm
  // Primordials to allow monkey-patching without affecting internals
  // Process object
}

MaybeLocal<Value> Realm::ExecuteBootstrapper(const char* id) {
  EscapableHandleScope scope(isolate());
  Local<Context> ctx = context();
  MaybeLocal<Value> result = env()->builtin_loader()->CompileAndCall(ctx, id, this);
  return scope.EscapeMaybe(result);
}

MaybeLocal<Value> Realm::RunBootstrapping() {
  EscapableHandleScope scope(isolate());
  Local<Value> result;
  if (!ExecuteBootstrapper("internal/bootstrap/realm").ToLocal(&result) || BootstrapRealm().ToLocal(&result)) {
    return MaybeLocal<Value>();
  }
  return scope.Escape(result);
}

Environment* Realm::env() const {
  return env_;
}

IsolateData* Realm::isolate_data() const {
  return env_->isolate_data();
}

Isolate* Realm::isolate() const {
  return isolate_;
}

// TODO: move to process._debugLog
static void DebugLogCallback(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  HandleScope handle_scope(isolate);
  String::Utf8Value utf8(isolate, args[0]);
  if (!*utf8) return;
  FILE* out = GetStdout();
  fprintf(out, "%s\n", *utf8);
  fflush(out);
}

PrincipalRealm::PrincipalRealm(Environment* env) : Realm(env) {
  CreateProperties();
  InitializeContext();
}

PrincipalRealm::~PrincipalRealm() {}

#define V(PropertyName, TypeName)                                                                                      \
  Local<TypeName> PrincipalRealm::PropertyName() const {                                                               \
    return PropertyName##_.Get(isolate());                                                                             \
  }                                                                                                                    \
  void PrincipalRealm::set_##PropertyName(Local<TypeName> value) {                                                     \
    PropertyName##_.Reset(isolate(), value);                                                                           \
  }
PER_REALM_STRONG_PERSISTENT_VALUES(V)
#undef V

MaybeLocal<Value> PrincipalRealm::BootstrapRealm() {
  HandleScope scope(isolate());

  if (ExecuteBootstrapper("internal/bootstrap/nyx").IsEmpty()) {
    return MaybeLocal<Value>();
  }

  return True(isolate());
}

void PrincipalRealm::InitializeContext() {
  Isolate::Scope isolate_scope(isolate_);
  HandleScope handle_scope(isolate_);

  // Create global template with builtins
  Local<ObjectTemplate> global_tmpl = ObjectTemplate::New(isolate_);
  SetMethod(isolate_, global_tmpl, "debugLog", DebugLogCallback);

  // Create context with global template
  Local<Context> context = Context::New(isolate_, nullptr, global_tmpl);
  context->SetAlignedPointerInEmbedderData(0, env());
  SetContext(context);
}

}  // namespace nyx
