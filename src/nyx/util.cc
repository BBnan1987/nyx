#include "util.h"

#include <simdutf.h>

namespace nyx {

using v8::ArrayBufferView;
using v8::Isolate;
using v8::Local;
using v8::String;
using v8::Value;

void Assert(const AssertionInfo& info) {
  fprintf(stderr,
          "\n"
          "  # %s at %s\n"
          "  # Assertion failed: %s\n",
          info.where ? info.where : "(unknown location)",
          info.file_line ? info.file_line : "(unknown source location)",
          info.what);
  ABORT();
}

template <typename T>
static void MakeUtf8String(Isolate* isolate, Local<Value> value, MaybeStackBuffer<T>* target) {
  Local<String> string;
  if (!value->ToString(isolate->GetCurrentContext()).ToLocal(&string)) return;
  String::ValueView value_view(isolate, string);

  auto value_length = value_view.length();

  if (value_view.is_one_byte()) {
    auto const_char = reinterpret_cast<const char*>(value_view.data8());
    auto expected_length = target->capacity() < (static_cast<size_t>(value_length) * 2 + 1)
                               ? simdutf::utf8_length_from_latin1(const_char, value_length)
                               : value_length * 2;

    target->AllocateSufficientStorage(expected_length + 1);
    const auto actual_length = simdutf::convert_latin1_to_utf8(const_char, value_length, target->out());
    target->SetLengthAndZeroTerminate(actual_length);
    return;
  }

  size_t storage = (3 * value_length) + 1;
  target->AllocateSufficientStorage(storage);

  size_t length =
      string->WriteUtf8(isolate, target->out(), static_cast<int>(storage), nullptr, String::REPLACE_INVALID_UTF8);
  target->SetLengthAndZeroTerminate(length);
}

Utf8Value::Utf8Value(Isolate* isolate, Local<Value> value) {
  if (value.IsEmpty()) return;

  MakeUtf8String(isolate, value, this);
}

TwoByteValue::TwoByteValue(Isolate* isolate, Local<Value> value) {
  if (value.IsEmpty()) {
    return;
  }

  Local<String> string;
  if (!value->ToString(isolate->GetCurrentContext()).ToLocal(&string)) return;

  // Allocate enough space to include the null terminator.
  const size_t length = string->Length();
  AllocateSufficientStorage(length + 1);
  string->Write(isolate, out(), 0, static_cast<int>(length));
  SetLengthAndZeroTerminate(length);
}

BufferValue::BufferValue(Isolate* isolate, Local<Value> value) {
  // Slightly different take on Utf8Value. If value is a String,
  // it will return a Utf8 encoded string. If value is a Buffer,
  // it will copy the data out of the Buffer as is.
  if (value.IsEmpty()) {
    // Dereferencing this object will return nullptr.
    Invalidate();
    return;
  }

  if (value->IsString()) {
    MakeUtf8String(isolate, value, this);
  } else if (value->IsArrayBufferView()) {
    const size_t len = value.As<ArrayBufferView>()->ByteLength();
    // Leave place for the terminating '\0' byte.
    AllocateSufficientStorage(len + 1);
    value.As<ArrayBufferView>()->CopyContents(out(), len);
    SetLengthAndZeroTerminate(len);
  } else {
    Invalidate();
  }
}

}  // namespace nyx
