#pragma once

#include <v8.h>
#include <memory>
#include <string>

namespace nyx {

template <typename Char, typename IChar, typename Base>
class StaticExternalByteResource : public Base {
 public:
  explicit StaticExternalByteResource(const Char* data, size_t length, std::shared_ptr<void> owning_ptr)
      : data_(data), length_(length), owning_ptr_(owning_ptr) {}

  const IChar* data() const override { return reinterpret_cast<const IChar*>(data_); }

  size_t length() const override { return length_; }
  void Dispose() override {}

  StaticExternalByteResource(const StaticExternalByteResource&) = delete;
  StaticExternalByteResource& operator=(const StaticExternalByteResource&) = delete;

 private:
  const Char* data_;
  const size_t length_;
  std::shared_ptr<void> owning_ptr_;
};

using StaticExternalOneByteResource =
    StaticExternalByteResource<uint8_t, char, v8::String::ExternalOneByteStringResource>;
using StaticExternalTwoByteResource =
    StaticExternalByteResource<uint16_t, uint16_t, v8::String::ExternalStringResource>;

class UnionBytes {
 public:
  explicit UnionBytes(StaticExternalOneByteResource* one_byte_resource)
      : one_byte_resource_(one_byte_resource), two_byte_resource_(nullptr) {}
  explicit UnionBytes(StaticExternalTwoByteResource* two_byte_resource)
      : one_byte_resource_(nullptr), two_byte_resource_(two_byte_resource) {}

  // Constructor for dynamic strings (used by external builtins)
  explicit UnionBytes(const std::string& source) : one_byte_resource_(nullptr), two_byte_resource_(nullptr) {
    // Create a shared_ptr to own the string data
    auto owned_data = std::make_shared<std::string>(source);
    one_byte_resource_ = new StaticExternalOneByteResource(
        reinterpret_cast<const uint8_t*>(owned_data->data()), owned_data->size(), owned_data);
  }

  UnionBytes() : one_byte_resource_(nullptr), two_byte_resource_(nullptr) {}

  UnionBytes(const UnionBytes&) = default;
  UnionBytes& operator=(const UnionBytes&) = default;
  UnionBytes(UnionBytes&&) = default;
  UnionBytes& operator=(UnionBytes&&) = default;

  bool is_one_byte() const { return one_byte_resource_ != nullptr; }

  v8::Local<v8::String> ToStringChecked(v8::Isolate* isolate) const {
    if (is_one_byte()) {
      return v8::String::NewExternalOneByte(isolate, one_byte_resource_).ToLocalChecked();
    } else {
      return v8::String::NewExternalTwoByte(isolate, two_byte_resource_).ToLocalChecked();
    }
  }

 private:
  StaticExternalOneByteResource* one_byte_resource_;
  StaticExternalTwoByteResource* two_byte_resource_;
};

}  // namespace nyx
