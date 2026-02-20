#pragma once

#include "nyx/base_object.h"

#include <uv.h>

#include <unordered_map>

namespace nyx {

class Environment;
class IsolateData;

class TimerWrap : public BaseObject {
 public:
  void Schedule(uint64_t timeout, uint64_t repeat);

  void Close();

  void CloseImmediate();

  uint64_t id() const { return id_; }

  void OnGCCollect() override;

 private:
  friend class TimerRegistry;
  TimerWrap(Realm* realm, v8::Local<v8::Object> obj, v8::Local<v8::Function> callback);
  ~TimerWrap() override;

  static void OnTimeout(uv_timer_t* handle);
  static void OnUvClose(uv_handle_t* handle);

  uv_timer_t handle_;
  v8::Global<v8::Function> callback_;
  uint64_t id_;
  uint64_t repeat_;

  static uint64_t next_id_;
};

class ImmediateWrap : public BaseObject {
 public:
  void Schedule();
  void Close();
  void CloseImmediate();

  uint64_t id() const { return id_; }

  void OnGCCollect() override;

 private:
  friend class TimerRegistry;
  ImmediateWrap(Realm* realm, v8::Local<v8::Object> obj, v8::Local<v8::Function> callback);
  ~ImmediateWrap() override;

  static void OnCheck(uv_check_t* handle);
  static void OnUvClose(uv_handle_t* handle);

  uv_check_t handle_;
  v8::Global<v8::Function> callback_;
  uint64_t id_;

  static uint64_t next_id_;
};

class TimerRegistry {
 public:
  explicit TimerRegistry(Environment* env);
  ~TimerRegistry();

  uint64_t CreateTimeout(Realm* realm, v8::Local<v8::Function> callback, uint64_t delay);
  uint64_t CreateInterval(Realm* realm, v8::Local<v8::Function> callback, uint64_t interval);
  uint64_t CreateImmediate(Realm* realm, v8::Local<v8::Function> callback);

  void CancelTimer(uint64_t id);
  void CancelImmediate(uint64_t id);

  void UnregisterTimer(uint64_t id);
  void UnregisterImmediate(uint64_t id);

  void CloseAll();

 private:
  Environment* env_;
  std::unordered_map<uint64_t, TimerWrap*> timers_;
  std::unordered_map<uint64_t, ImmediateWrap*> immediates_;
};

}  // namespace nyx
