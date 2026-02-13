#pragma once

#include "nyx/util.h"

namespace nyx {

class Environment;
class Realm;

class BaseObject {
 public:
  enum InternalFields {
    kSlot,
    kInternalFieldCount,
  };

  BaseObject(Realm* realm, v8::Local<v8::Object> object);
  virtual ~BaseObject();

  // prevent copying
  BaseObject(const BaseObject&) = delete;
  BaseObject& operator=(const BaseObject&) = delete;

  v8::Local<v8::Object> object() const;
  v8::Global<v8::Object>& persistent();

  Environment* env() const;
  Realm* realm() const;
  v8::Isolate* isolate() const;

  // Make the persistent handle weak, so the C++ object gets deleted when
  // the JS object is garbage collected
  void MakeWeak();

  // Prevent GC from collecting this object
  void ClearWeak();

  virtual void OnGCCollect();

  static BaseObject* FromJSObject(v8::Local<v8::Value> object);
  template <typename T>
  static inline T* FromJSObject(v8::Local<v8::Value> object);
  // Retrieve the C++ object from a JS object's internal field
  template <typename T>
  static T* Unwrap(v8::Local<v8::Value> object) {
    return BaseObject::FromJSObject<T>(object);
  }

 private:
  Realm* realm_;
  v8::Global<v8::Object> persistent_handle_;
};

template <typename T>
inline T* BaseObject::FromJSObject(v8::Local<v8::Value> object) {
  return static_cast<T*>(FromJSObject(object));
}

#define ASSIGN_OR_RETURN_UNWRAP(ptr, obj, ...)                                                                         \
  do {                                                                                                                 \
    *ptr = static_cast<typename std::remove_reference<decltype(*ptr)>::type>(BaseObject::FromJSObject(obj));           \
    if (*ptr == nullptr) {                                                                                             \
      return __VA_ARGS__;                                                                                              \
    }                                                                                                                  \
  } while (0)

}  // namespace nyx
