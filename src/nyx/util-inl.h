#pragma once

#include "nyx/util.h"

namespace nyx {

struct ArrayIterationData {
  std::vector<v8::Global<v8::Value>>* out;
  v8::Isolate* isolate = nullptr;
};

inline v8::Array::CallbackResult PushItemToVector(uint32_t /* index*/, v8::Local<v8::Value> element, void* data) {
  auto vec = static_cast<ArrayIterationData*>(data)->out;
  auto isolate = static_cast<ArrayIterationData*>(data)->isolate;
  vec->push_back(v8::Global<v8::Value>(isolate, element));
  return v8::Array::CallbackResult::kContinue;
}

v8::Maybe<void> FromV8Array(v8::Local<v8::Context> context,
                            v8::Local<v8::Array> js_array,
                            std::vector<v8::Global<v8::Value>>* out) {
  uint32_t count = js_array->Length();
  out->reserve(count);
  ArrayIterationData data{out, context->GetIsolate()};
  return js_array->Iterate(context, PushItemToVector, &data);
}

v8::MaybeLocal<v8::Value> ToV8Value(v8::Local<v8::Context> context, std::string_view str, v8::Isolate* isolate) {
  if (isolate == nullptr) {
    isolate = context->GetIsolate();
  }
  if (str.size() >= static_cast<size_t>(v8::String::kMaxLength)) [[unlikely]] {
    isolate->ThrowError("String too long");
    return v8::MaybeLocal<v8::Value>();
  }
  return v8::String::NewFromUtf8(isolate, str.data(), v8::NewStringType::kNormal, static_cast<int>(str.size()))
      .FromMaybe(v8::Local<v8::String>());
}

template <typename T>
v8::Local<v8::Value> ConvertNumberToV8Value(v8::Isolate* isolate, const T& number) {
  using Limits = std::numeric_limits<T>;
  if (static_cast<uint32_t>(Limits::max()) <= std::numeric_limits<uint32_t>::max() &&
      static_cast<uint32_t>(Limits::min()) >= std::numeric_limits<uint32_t>::min() && Limits::is_exact) {
    return v8::Integer::NewFromUnsigned(isolate, static_cast<uint32_t>(number));
  }
  if (static_cast<int32_t>(Limits::max()) <= std::numeric_limits<int32_t>::max() &&
      static_cast<int32_t>(Limits::min()) >= std::numeric_limits<int32_t>::min() && Limits::is_exact) {
    return v8::Integer::New(isolate, static_cast<int32_t>(number));
  }
  return v8::Number::New(isolate, static_cast<double>(number));
}

template <typename T, typename test_for_number>
v8::MaybeLocal<v8::Value> ToV8Value(v8::Local<v8::Context> context, const T& number, v8::Isolate* isolate) {
  if (isolate == nullptr) {
    isolate = context->GetIsolate();
  }
  return ConvertNumberToV8Value(isolate, number);
}

template <typename T>
v8::MaybeLocal<v8::Value> ToV8Value(v8::Local<v8::Context> context, const std::vector<T>& vec, v8::Isolate* isolate) {
  if (isolate == nullptr) {
    isolate = context->GetIsolate();
  }
  v8::EscapableHandleScope handle_scope(isolate);

  MaybeStackBuffer<v8::Local<v8::Value>, 128> arr(vec.size());
  arr.SetLength(vec.size());
  for (size_t i = 0; i < vec.size(); ++i) {
    if (!ToV8Value(context, vec[i], isolate).ToLocal(&arr[i])) {
      return v8::MaybeLocal<v8::Value>();
    }
  }

  return handle_scope.Escape(v8::Array::New(isolate, arr.out(), arr.length()));
}

template <typename T>
v8::MaybeLocal<v8::Value> ToV8Value(v8::Local<v8::Context> context, const std::set<T>& set, v8::Isolate* isolate) {
  if (isolate == nullptr) {
    isolate = context->GetIsolate();
  }
  v8::Local<v8::Set> set_js = v8::Set::New(isolate);
  v8::HandleScope handle_scope(isolate);

  for (const T& entry : set) {
    v8::Local<v8::Value> value;
    if (!ToV8Value(context, entry, isolate).ToLocal(&value)) {
      return {};
    }
    if (set_js->Add(context, value).IsEmpty()) {
      return {};
    }
  }

  return set_js;
}

template <typename T, typename U>
v8::MaybeLocal<v8::Value> ToV8Value(v8::Local<v8::Context> context,
                                    const std::unordered_map<T, U>& map,
                                    v8::Isolate* isolate) {
  if (isolate == nullptr) {
    isolate = context->GetIsolate();
  }
  v8::EscapableHandleScope handle_scope(isolate);

  v8::Local<v8::Map> ret = v8::Map::New(isolate);
  for (const auto& item : map) {
    v8::Local<v8::Value> first, second;
    if (!ToV8Value(context, item.first, isolate).ToLocal(&first) ||
        !ToV8Value(context, item.second, isolate).ToLocal(&second) || ret->Set(context, first, second).IsEmpty()) {
      return v8::MaybeLocal<v8::Value>();
    }
  }

  return handle_scope.Escape(ret);
}

template <typename T, std::size_t U>
v8::MaybeLocal<v8::Value> ToV8Value(v8::Local<v8::Context> context,
                                    const std::ranges::elements_view<T, U>& vec,
                                    v8::Isolate* isolate) {
  if (isolate == nullptr) {
    isolate = context->GetIsolate();
  }
  v8::EscapableHandleScope handle_scope(isolate);

  MaybeStackBuffer<v8::Local<v8::Value>, 128> arr(vec.size());
  arr.SetLength(vec.size());
  auto it = vec.begin();
  for (size_t i = 0; i < vec.size(); ++i) {
    if (!ToV8Value(context, *it, isolate).ToLocal(&arr[i])) {
      return v8::MaybeLocal<v8::Value>();
    }
    std::advance(it, 1);
  }

  return handle_scope.Escape(v8::Array::New(isolate, arr.out(), arr.length()));
}

}  // namespace nyx
