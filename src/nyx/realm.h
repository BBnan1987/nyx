#pragma once

#include "nyx/isolate_data.h"
#include "nyx/util.h"

#include <set>

namespace nyx {

class Environment;
class IsolateData;

class Realm {
 public:
  Realm(Environment* env, v8::Local<v8::Context> context);

  static Realm* GetCurrent(v8::Isolate* isolate);
  static Realm* GetCurrent(v8::Local<v8::Context> context);
  static Realm* GetCurrent(const v8::FunctionCallbackInfo<v8::Value>& args);
  template <typename T>
  inline static Realm* GetCurrent(const v8::PropertyCallbackInfo<T>& args) {
    return GetCurrent(args.GetIsolate()->GetCurrentContext());
  }

  void CreateProperties();

  v8::MaybeLocal<v8::Value> ExecuteBootstrapper(const char* id);
  v8::MaybeLocal<v8::Value> RunBootstrapping();

  Environment* env() const;
  IsolateData* isolate_data() const;
  v8::Isolate* isolate() const;
  v8::Local<v8::Context> context() const { return context_.Get(isolate()); }
  v8::Local<v8::Object> global() const { return context()->Global(); }

#define V(PropertyName, TypeName)                                                                                      \
  virtual v8::Local<TypeName> PropertyName() const = 0;                                                                \
  virtual void set_##PropertyName(v8::Local<TypeName> value) = 0;
  PER_REALM_STRONG_PERSISTENT_VALUES(V)
#undef V

  // context aware internal bindings
  std::set<struct InternalBinding*> internal_bindings;

 protected:
  Realm(Environment* env);
  ~Realm();
  void SetContext(v8::Local<v8::Context> context);

  virtual v8::MaybeLocal<v8::Value> BootstrapRealm() = 0;

  Environment* env_;
  v8::Isolate* isolate_;
  v8::Global<v8::Context> context_;

#define V(PropertyName, TypeName) v8::Global<TypeName> PropertyName##_;
  PER_REALM_STRONG_PERSISTENT_VALUES(V)
#undef V
};

class PrincipalRealm : public Realm {
 public:
  PrincipalRealm(Environment* env);
  ~PrincipalRealm();

#define V(PropertyName, TypeName)                                                                                      \
  v8::Local<TypeName> PropertyName() const override;                                                                   \
  void set_##PropertyName(v8::Local<TypeName> value) override;
  PER_REALM_STRONG_PERSISTENT_VALUES(V)
#undef V

 protected:
  v8::MaybeLocal<v8::Value> BootstrapRealm() override;

 private:
  void InitializeContext();
};

}  // namespace nyx
