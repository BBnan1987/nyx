#include "nyx/timers.h"

#include "nyx/env.h"
#include "nyx/nyx_binding.h"
#include "nyx/util.h"

namespace nyx {

using v8::Context;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::HandleScope;
using v8::Integer;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::ObjectTemplate;
using v8::Value;

// Static ID counters
uint64_t TimerWrap::next_id_ = 1;
uint64_t ImmediateWrap::next_id_ = 1;

// Global timer registry (one per process for now)
static TimerRegistry* g_timer_registry = nullptr;

TimerRegistry& GetTimerRegistry() {
  if (g_timer_registry == nullptr) {
    g_timer_registry = new TimerRegistry();
  }
  return *g_timer_registry;
}

// ============================================================================
// TimerWrap implementation
// ============================================================================

TimerWrap::TimerWrap(Environment* env, Local<Function> callback, uint64_t timeout, uint64_t repeat)
    : env_(env), timeout_(timeout), repeat_(repeat), id_(next_id_++) {
  callback_.Reset(env->isolate(), callback);
  uv_timer_init(env->event_loop(), &handle_);
  handle_.data = this;
}

TimerWrap::~TimerWrap() {
  callback_.Reset();
}

void TimerWrap::Start() {
  uv_timer_start(&handle_, OnTimeout, timeout_, repeat_);
}

void TimerWrap::Stop() {
  uv_timer_stop(&handle_);
}

void TimerWrap::Close() {
  Stop();
  uv_close(reinterpret_cast<uv_handle_t*>(&handle_), OnClose);
}

void TimerWrap::OnClose(uv_handle_t* handle) {
  TimerWrap* wrap = static_cast<TimerWrap*>(handle->data);
  delete wrap;
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

  Local<Function> callback = wrap->callback_.Get(isolate);
  if (!callback.IsEmpty()) {
    TryCatchScope try_catch(isolate);
    callback->Call(context, context->Global(), 0, nullptr);
  }

  // If this is a one-shot timer (setTimeout), close it
  if (is_one_shot) {
    GetTimerRegistry().UnregisterTimer(id);
    wrap->Close();
  }
}

// ============================================================================
// ImmediateWrap implementation
// ============================================================================

ImmediateWrap::ImmediateWrap(Environment* env, Local<Function> callback) : env_(env), id_(next_id_++) {
  callback_.Reset(env->isolate(), callback);
  uv_check_init(env->event_loop(), &handle_);
  handle_.data = this;
}

ImmediateWrap::~ImmediateWrap() {
  callback_.Reset();
}

void ImmediateWrap::Start() {
  uv_check_start(&handle_, OnCheck);
}

void ImmediateWrap::Stop() {
  uv_check_stop(&handle_);
}

void ImmediateWrap::Close() {
  Stop();
  uv_close(reinterpret_cast<uv_handle_t*>(&handle_), OnClose);
}

void ImmediateWrap::OnClose(uv_handle_t* handle) {
  ImmediateWrap* wrap = static_cast<ImmediateWrap*>(handle->data);
  delete wrap;
}

void ImmediateWrap::OnCheck(uv_check_t* handle) {
  ImmediateWrap* wrap = static_cast<ImmediateWrap*>(handle->data);

  // Immediates only fire once
  if (wrap->fired_) return;
  wrap->fired_ = true;

  Environment* env = wrap->env();
  Isolate* isolate = env->isolate();
  uint64_t id = wrap->id();

  HandleScope handle_scope(isolate);
  Local<Context> context = env->context();
  Context::Scope context_scope(context);

  Local<Function> callback = wrap->callback_.Get(isolate);
  if (!callback.IsEmpty()) {
    TryCatchScope try_catch(isolate);
    callback->Call(context, context->Global(), 0, nullptr);
  }

  // Unregister and close after firing
  GetTimerRegistry().UnregisterImmediate(id);
  wrap->Close();
}

// ============================================================================
// TimerRegistry implementation
// ============================================================================

TimerRegistry::~TimerRegistry() {
  // Note: At shutdown, we just clear the maps. The handles should already
  // be closed by the event loop cleanup. If not, this could leak, but
  // it's process exit anyway.
  timers_.clear();
  immediates_.clear();
}

uint64_t TimerRegistry::SetTimeout(Environment* env, Local<Function> callback, uint64_t delay) {
  TimerWrap* timer = new TimerWrap(env, callback, delay, 0);
  timers_[timer->id()] = timer;
  timer->Start();
  return timer->id();
}

uint64_t TimerRegistry::SetInterval(Environment* env, Local<Function> callback, uint64_t interval) {
  TimerWrap* timer = new TimerWrap(env, callback, interval, interval);
  timers_[timer->id()] = timer;
  timer->Start();
  return timer->id();
}

void TimerRegistry::ClearTimer(uint64_t id) {
  auto it = timers_.find(id);
  if (it != timers_.end()) {
    TimerWrap* timer = it->second;
    timers_.erase(it);
    timer->Close();
  }
}

uint64_t TimerRegistry::SetImmediate(Environment* env, Local<Function> callback) {
  ImmediateWrap* immediate = new ImmediateWrap(env, callback);
  immediates_[immediate->id()] = immediate;
  immediate->Start();
  return immediate->id();
}

void TimerRegistry::ClearImmediate(uint64_t id) {
  auto it = immediates_.find(id);
  if (it != immediates_.end()) {
    ImmediateWrap* immediate = it->second;
    immediates_.erase(it);
    immediate->Close();
  }
}

void TimerRegistry::UnregisterTimer(uint64_t id) {
  timers_.erase(id);
}

void TimerRegistry::UnregisterImmediate(uint64_t id) {
  immediates_.erase(id);
}

// ============================================================================
// JS Callbacks
// ============================================================================

static void SetTimeoutCallback(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);

  if (args.Length() < 1 || !args[0]->IsFunction()) {
    isolate->ThrowException(OneByteString(isolate, "Callback must be a function"));
    return;
  }

  Local<Function> callback = args[0].As<Function>();
  uint64_t delay = 0;
  if (args.Length() > 1 && args[1]->IsNumber()) {
    delay = static_cast<uint64_t>(args[1]->NumberValue(context).FromMaybe(0));
  }
  if (delay < 1) {
    delay = 1;
  }

  uint64_t id = GetTimerRegistry().SetTimeout(env, callback, delay);
  args.GetReturnValue().Set(Number::New(isolate, static_cast<double>(id)));
}

static void SetIntervalCallback(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);

  if (args.Length() < 1 || !args[0]->IsFunction()) {
    isolate->ThrowException(OneByteString(isolate, "Callback must be a function"));
    return;
  }

  Local<Function> callback = args[0].As<Function>();
  uint64_t interval = 1;
  if (args.Length() > 1 && args[1]->IsNumber()) {
    interval = static_cast<uint64_t>(args[1]->NumberValue(context).FromMaybe(0));
  }
  if (interval < 1) {
    interval = 1;
  }

  uint64_t id = GetTimerRegistry().SetInterval(env, callback, interval);
  args.GetReturnValue().Set(Number::New(isolate, static_cast<double>(id)));
}

static void ClearTimeoutCallback(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();

  if (args.Length() < 1 || !args[0]->IsNumber()) {
    return;  // Silently ignore invalid arguments like browsers do
  }

  uint64_t id = static_cast<uint64_t>(args[0]->NumberValue(context).FromMaybe(0));
  GetTimerRegistry().ClearTimer(id);
}

static void SetImmediateCallback(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);

  if (args.Length() < 1 || !args[0]->IsFunction()) {
    isolate->ThrowException(OneByteString(isolate, "Callback must be a function"));
    return;
  }

  Local<Function> callback = args[0].As<Function>();
  uint64_t id = GetTimerRegistry().SetImmediate(env, callback);
  args.GetReturnValue().Set(Number::New(isolate, static_cast<double>(id)));
}

static void ClearImmediateCallback(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();

  if (args.Length() < 1 || !args[0]->IsNumber()) {
    return;  // Silently ignore invalid arguments
  }

  uint64_t id = static_cast<uint64_t>(args[0]->NumberValue(context).FromMaybe(0));
  GetTimerRegistry().ClearImmediate(id);
}

// ============================================================================
// Binding Registration
// ============================================================================

static void CreatePerIsolateProperties(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();

  SetMethod(isolate, target, "setTimeout", SetTimeoutCallback);
  SetMethod(isolate, target, "setInterval", SetIntervalCallback);
  SetMethod(isolate, target, "clearTimeout", ClearTimeoutCallback);
  SetMethod(isolate, target, "clearInterval", ClearTimeoutCallback);  // Same as clearTimeout
  SetMethod(isolate, target, "setImmediate", SetImmediateCallback);
  SetMethod(isolate, target, "clearImmediate", ClearImmediateCallback);
}

static void CreatePerContextProperties(Local<Object> target, Local<Context> context) {}

NYX_BINDING_PER_ISOLATE_INIT(timers, CreatePerIsolateProperties)
NYX_BINDING_CONTEXT_AWARE(timers, CreatePerContextProperties)

}  // namespace nyx
