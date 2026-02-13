#include "nyx/base_object.h"

#include "nyx/env.h"
#include "nyx/realm.h"

namespace nyx {

using v8::Global;
using v8::HandleScope;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::Value;
using v8::WeakCallbackInfo;
using v8::WeakCallbackType;

BaseObject::BaseObject(Realm* realm, Local<Object> object)
    : realm_(realm), persistent_handle_(realm->isolate(), object) {
  object->SetAlignedPointerInInternalField(kSlot, this);
}

BaseObject::~BaseObject() {
  if (persistent_handle_.IsEmpty()) {
    return;
  }
  {
    HandleScope handle_scope(env()->isolate());
    object()->SetAlignedPointerInInternalField(kSlot, nullptr);
  }
}

Local<Object> BaseObject::object() const {
  return persistent_handle_.Get(isolate());
}

Global<Object>& BaseObject::persistent() {
  return persistent_handle_;
}

Environment* BaseObject::env() const {
  return realm_->env();
}

Realm* BaseObject::realm() const {
  return realm_;
}

Isolate* BaseObject::isolate() const {
  return realm_->isolate();
}

void BaseObject::MakeWeak() {
  if (persistent_handle_.IsWeak()) {
    return;
  }
  persistent_handle_.SetWeak(
      this,
      [](const WeakCallbackInfo<BaseObject>& data) {
        BaseObject* obj = data.GetParameter();
        obj->persistent_handle_.Reset();
        obj->OnGCCollect();
      },
      WeakCallbackType::kParameter);
}

void BaseObject::ClearWeak() {
  persistent_handle_.ClearWeak();
}

void BaseObject::OnGCCollect() {
  delete this;
}

BaseObject* BaseObject::FromJSObject(Local<Value> object) {
  if (!object->IsObject()) {
    return nullptr;
  }
  Local<Object> obj = object.As<Object>();
  return static_cast<BaseObject*>(obj->GetAlignedPointerFromInternalField(kSlot));
}

}  // namespace nyx
