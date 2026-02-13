#pragma once

#include <uv.h>
#include <v8.h>

#include <unordered_map>

namespace nyx {

class Environment;

class TimerWrap {
 public:
  TimerWrap(Environment* env, v8::Local<v8::Function> callback, uint64_t timeout, uint64_t repeat);
  ~TimerWrap();

  void Start();
  void Stop();
  void Close();

  uint64_t id() const { return id_; }
  Environment* env() const { return env_; }

  static void OnTimeout(uv_timer_t* handle);
  static void OnClose(uv_handle_t* handle);

 private:
  Environment* env_;
  uv_timer_t handle_;
  v8::Global<v8::Function> callback_;
  uint64_t timeout_;
  uint64_t repeat_;
  uint64_t id_;

  static uint64_t next_id_;
};

class ImmediateWrap {
 public:
  ImmediateWrap(Environment* env, v8::Local<v8::Function> callback);
  ~ImmediateWrap();

  void Start();
  void Stop();
  void Close();

  uint64_t id() const { return id_; }
  Environment* env() const { return env_; }

  static void OnCheck(uv_check_t* handle);
  static void OnClose(uv_handle_t* handle);

 private:
  Environment* env_;
  uv_check_t handle_;
  v8::Global<v8::Function> callback_;
  uint64_t id_;
  bool fired_ = false;

  static uint64_t next_id_;
};

class TimerRegistry {
 public:
  TimerRegistry() = default;
  ~TimerRegistry();

  uint64_t SetTimeout(Environment* env, v8::Local<v8::Function> callback, uint64_t delay);
  uint64_t SetInterval(Environment* env, v8::Local<v8::Function> callback, uint64_t interval);

  void ClearTimer(uint64_t id);

  uint64_t SetImmediate(Environment* env, v8::Local<v8::Function> callback);
  void ClearImmediate(uint64_t id);

  void UnregisterTimer(uint64_t id);
  void UnregisterImmediate(uint64_t id);

 private:
  std::unordered_map<uint64_t, TimerWrap*> timers_;
  std::unordered_map<uint64_t, ImmediateWrap*> immediates_;
};

}  // namespace nyx
