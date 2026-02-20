#include "nyx/timers.h"

#include "nyx/env.h"
#include "nyx/isolate_data.h"
#include "nyx/nyx_binding.h"
#include "nyx/realm.h"
#include "nyx/util.h"

namespace nyx {

using v8::Context;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::HandleScope;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::ObjectTemplate;
using v8::Value;

uint64_t TimerWrap::next_id_ = 1;

TimerWrap::TimerWrap(Realm* realm, Local<Object> obj, Local<Function> callback)
    : BaseObject(realm, obj), id_(next_id_++), repeat_(0) {
  callback_.Reset(isolate(), callback);
  uv_timer_init(env()->event_loop(), &handle_);
  handle_.data = this;
  // Hold the object strongly while the timer is active.
  ClearWeak();
}

TimerWrap::~TimerWrap() {
  callback_.Reset();
}

void TimerWrap::Schedule(uint64_t timeout, uint64_t repeat) {
  repeat_ = repeat;
  uv_timer_start(&handle_, OnTimeout, timeout, repeat);
}

void TimerWrap::Close() {
  uv_timer_stop(&handle_);
  uv_close(reinterpret_cast<uv_handle_t*>(&handle_), OnUvClose);
}

void TimerWrap::CloseImmediate() {
  callback_.Reset();
  uv_timer_stop(&handle_);
  uv_close(reinterpret_cast<uv_handle_t*>(&handle_), OnUvClose);
}

void TimerWrap::OnGCCollect() {
  delete this;
}

void TimerWrap::OnTimeout(uv_timer_t* handle) {
  TimerWrap* wrap = static_cast<TimerWrap*>(handle->data);
  Environment* env = wrap->env();
  Isolate* isolate = env->isolate();
  uint64_t id = wrap->id();
  bool is_one_shot = (wrap->repeat_ == 0);

  HandleScope handle_scope(isolate);
  Local<Context> context = env->context();
  Context::Scope context_scope(context);

  if (!wrap->callback_.IsEmpty()) {
    Local<Function> callback = wrap->callback_.Get(isolate);
    TryCatchScope try_catch(isolate);
    callback->Call(context, context->Global(), 0, nullptr);
  }

  if (is_one_shot) {
    env->timer_registry().UnregisterTimer(id);
    wrap->Close();
  }
}

void TimerWrap::OnUvClose(uv_handle_t* handle) {
  TimerWrap* wrap = static_cast<TimerWrap*>(handle->data);
  wrap->MakeWeak();
}

uint64_t ImmediateWrap::next_id_ = 1;

ImmediateWrap::ImmediateWrap(Realm* realm, Local<Object> obj, Local<Function> callback)
    : BaseObject(realm, obj), id_(next_id_++) {
  callback_.Reset(isolate(), callback);
  uv_check_init(env()->event_loop(), &handle_);
  handle_.data = this;
  ClearWeak();
}

ImmediateWrap::~ImmediateWrap() {
  callback_.Reset();
}

void ImmediateWrap::Schedule() {
  uv_check_start(&handle_, OnCheck);
}

void ImmediateWrap::Close() {
  uv_check_stop(&handle_);
  uv_close(reinterpret_cast<uv_handle_t*>(&handle_), OnUvClose);
}

void ImmediateWrap::CloseImmediate() {
  callback_.Reset();
  uv_check_stop(&handle_);
  uv_close(reinterpret_cast<uv_handle_t*>(&handle_), OnUvClose);
}

void ImmediateWrap::OnGCCollect() {
  delete this;
}

void ImmediateWrap::OnCheck(uv_check_t* handle) {
  ImmediateWrap* wrap = static_cast<ImmediateWrap*>(handle->data);
  uv_check_stop(handle);

  Environment* env = wrap->env();
  Isolate* isolate = env->isolate();
  uint64_t id = wrap->id();

  HandleScope handle_scope(isolate);
  Local<Context> context = env->context();
  Context::Scope context_scope(context);

  if (!wrap->callback_.IsEmpty()) {
    Local<Function> callback = wrap->callback_.Get(isolate);
    TryCatchScope try_catch(isolate);
    callback->Call(context, context->Global(), 0, nullptr);
  }

  env->timer_registry().UnregisterImmediate(id);
  wrap->Close();
}

void ImmediateWrap::OnUvClose(uv_handle_t* handle) {
  ImmediateWrap* wrap = static_cast<ImmediateWrap*>(handle->data);
  wrap->MakeWeak();
}

TimerRegistry::TimerRegistry(Environment* env) : env_(env) {}

TimerRegistry::~TimerRegistry() {}

uint64_t TimerRegistry::CreateTimeout(Realm* realm, Local<Function> callback, uint64_t delay) {
  Local<Context> context = realm->context();
  Local<ObjectTemplate> tmpl = realm->isolate_data()->binding_data_default_template();
  Local<Object> obj;
  if (!tmpl->NewInstance(context).ToLocal(&obj)) return 0;

  auto* wrap = new TimerWrap(realm, obj, callback);
  uint64_t id = wrap->id();
  timers_[id] = wrap;
  wrap->Schedule(delay, 0);
  return id;
}

uint64_t TimerRegistry::CreateInterval(Realm* realm, Local<Function> callback, uint64_t interval) {
  Local<Context> context = realm->context();
  Local<ObjectTemplate> tmpl = realm->isolate_data()->binding_data_default_template();
  Local<Object> obj;
  if (!tmpl->NewInstance(context).ToLocal(&obj)) return 0;

  auto* wrap = new TimerWrap(realm, obj, callback);
  uint64_t id = wrap->id();
  timers_[id] = wrap;
  wrap->Schedule(interval, interval);
  return id;
}

uint64_t TimerRegistry::CreateImmediate(Realm* realm, Local<Function> callback) {
  Local<Context> context = realm->context();
  Local<ObjectTemplate> tmpl = realm->isolate_data()->binding_data_default_template();
  Local<Object> obj;
  if (!tmpl->NewInstance(context).ToLocal(&obj)) return 0;

  auto* wrap = new ImmediateWrap(realm, obj, callback);
  uint64_t id = wrap->id();
  immediates_[id] = wrap;
  wrap->Schedule();
  return id;
}

void TimerRegistry::CancelTimer(uint64_t id) {
  auto it = timers_.find(id);
  if (it == timers_.end()) return;
  TimerWrap* wrap = it->second;
  timers_.erase(it);
  wrap->Close();
}

void TimerRegistry::CancelImmediate(uint64_t id) {
  auto it = immediates_.find(id);
  if (it == immediates_.end()) return;
  ImmediateWrap* wrap = it->second;
  immediates_.erase(it);
  wrap->Close();
}

void TimerRegistry::UnregisterTimer(uint64_t id) {
  timers_.erase(id);
}

void TimerRegistry::UnregisterImmediate(uint64_t id) {
  immediates_.erase(id);
}

void TimerRegistry::CloseAll() {
  auto timers = std::move(timers_);
  auto immediates = std::move(immediates_);

  for (auto& [id, wrap] : timers) {
    wrap->CloseImmediate();
  }
  for (auto& [id, wrap] : immediates) {
    wrap->CloseImmediate();
  }
}

static void SetTimeoutCallback(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Realm* realm = Realm::GetCurrent(context);

  if (args.Length() < 1 || !args[0]->IsFunction()) {
    isolate->ThrowException(OneByteString(isolate, "Callback must be a function"));
    return;
  }

  Local<Function> callback = args[0].As<Function>();
  uint64_t delay = 1;
  if (args.Length() > 1 && args[1]->IsNumber()) {
    double v = args[1]->NumberValue(context).FromMaybe(0.0);
    if (v >= 1.0) delay = static_cast<uint64_t>(v);
  }

  uint64_t id = env->timer_registry().CreateTimeout(realm, callback, delay);
  args.GetReturnValue().Set(Number::New(isolate, static_cast<double>(id)));
}

static void SetIntervalCallback(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Realm* realm = Realm::GetCurrent(context);

  if (args.Length() < 1 || !args[0]->IsFunction()) {
    isolate->ThrowException(OneByteString(isolate, "Callback must be a function"));
    return;
  }

  Local<Function> callback = args[0].As<Function>();
  uint64_t interval = 1;
  if (args.Length() > 1 && args[1]->IsNumber()) {
    double v = args[1]->NumberValue(context).FromMaybe(0.0);
    if (v >= 1.0) interval = static_cast<uint64_t>(v);
  }

  uint64_t id = env->timer_registry().CreateInterval(realm, callback, interval);
  args.GetReturnValue().Set(Number::New(isolate, static_cast<double>(id)));
}

static void ClearTimeoutCallback(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);

  if (args.Length() < 1 || !args[0]->IsNumber()) return;
  uint64_t id = static_cast<uint64_t>(args[0]->NumberValue(context).FromMaybe(0.0));
  env->timer_registry().CancelTimer(id);
}

static void SetImmediateCallback(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);
  Realm* realm = Realm::GetCurrent(context);

  if (args.Length() < 1 || !args[0]->IsFunction()) {
    isolate->ThrowException(OneByteString(isolate, "Callback must be a function"));
    return;
  }

  Local<Function> callback = args[0].As<Function>();
  uint64_t id = env->timer_registry().CreateImmediate(realm, callback);
  args.GetReturnValue().Set(Number::New(isolate, static_cast<double>(id)));
}

static void ClearImmediateCallback(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);

  if (args.Length() < 1 || !args[0]->IsNumber()) return;
  uint64_t id = static_cast<uint64_t>(args[0]->NumberValue(context).FromMaybe(0.0));
  env->timer_registry().CancelImmediate(id);
}

static void CreatePerIsolateProperties(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();

  SetMethod(isolate, target, "setTimeout", SetTimeoutCallback);
  SetMethod(isolate, target, "setInterval", SetIntervalCallback);
  SetMethod(isolate, target, "clearTimeout", ClearTimeoutCallback);
  SetMethod(isolate, target, "clearInterval", ClearTimeoutCallback);
  SetMethod(isolate, target, "setImmediate", SetImmediateCallback);
  SetMethod(isolate, target, "clearImmediate", ClearImmediateCallback);
}

static void CreatePerContextProperties(Local<Object> target, Local<Context> context) {}

NYX_BINDING_PER_ISOLATE_INIT(timers, CreatePerIsolateProperties)
NYX_BINDING_CONTEXT_AWARE(timers, CreatePerContextProperties)

}  // namespace nyx
