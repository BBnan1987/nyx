#include "nyx/env.h"
#include "nyx/errors.h"
#include "nyx/game_lock.h"
#include "nyx/isolate_data.h"
#include "nyx/nyx_binding.h"
#include "nyx/util.h"

#include <Windows.h>
#include <nmmintrin.h>

#include <algorithm>
#include <chrono>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

namespace nyx {

struct MemCacheKey {
  uint64_t address;
  uint32_t size;
  bool operator==(const MemCacheKey& o) const {
    return address == o.address && size == o.size;
  }
};
struct MemCacheKeyHash {
  size_t operator()(const MemCacheKey& k) const {
    size_t h = std::hash<uint64_t>()(k.address);
    h ^= std::hash<uint32_t>()(k.size) + 0x9e3779b9u + (h << 6) + (h >> 2);
    return h;
  }
};
static std::unordered_map<MemCacheKey, uint32_t, MemCacheKeyHash> checksum_cache_;
static thread_local std::vector<uint8_t> read_buf_;

using v8::Array;
using v8::BigInt;
using v8::Context;
using v8::FunctionCallbackInfo;
using v8::HandleScope;
using v8::Integer;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::ObjectTemplate;
using v8::String;
using v8::Uint8Array;
using v8::Value;

static uint32_t Crc32Hash(const uint8_t* data, size_t size) {
  uint64_t crc = 0xFFFFFFFFull;
  size_t i = 0;
  for (; i + 8 <= size; i += 8) {
    uint64_t chunk;
    std::memcpy(&chunk, data + i, 8);
    crc = _mm_crc32_u64(crc, chunk);
  }
  for (; i < size; i++) {
    crc = _mm_crc32_u8(static_cast<uint32_t>(crc), data[i]);
  }
  return static_cast<uint32_t>(crc ^ 0xFFFFFFFFull);
}

static bool SafeMemcpy(void* dest, const void* src, size_t size) {
  __try {
    std::memcpy(dest, src, size);
    return true;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    return false;
  }
}

// readMemory(address: BigInt, size: number) -> Uint8Array
static void ReadMemory(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();

  uint64_t address = args[0]->IsBigInt() ? args[0].As<BigInt>()->Uint64Value()
                                         : static_cast<uint64_t>(args[0]->NumberValue(context).FromMaybe(0));

  size_t size = args[1]->Uint32Value(context).FromMaybe(0);

  auto backing_store = v8::ArrayBuffer::NewBackingStore(isolate, size);

  if (!SafeMemcpy(backing_store->Data(), reinterpret_cast<void*>(address), size)) {
    isolate->ThrowError("Access violation reading memory");
    return;
  }

  Local<v8::ArrayBuffer> ab = v8::ArrayBuffer::New(isolate, std::move(backing_store));
  args.GetReturnValue().Set(Uint8Array::New(ab, 0, size));
}

// readMemoryFast(address: BigInt, size: number) -> Uint8Array
static void ReadMemoryFast(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  uint64_t address = args[0].As<BigInt>()->Uint64Value();
  size_t size = static_cast<size_t>(args[1].As<v8::Uint32>()->Value());

  auto backing_store = v8::ArrayBuffer::NewBackingStore(isolate, size);

  if (!SafeMemcpy(backing_store->Data(), reinterpret_cast<void*>(address), size)) {
    isolate->ThrowError("Access violation reading memory");
    return;
  }

  Local<v8::ArrayBuffer> ab = v8::ArrayBuffer::New(isolate, std::move(backing_store));
  args.GetReturnValue().Set(Uint8Array::New(ab, 0, size));
}

// readMemoryIfChanged(address: BigInt, size: Uint32) -> Uint8Array | undefined
static void ReadMemoryIfChanged(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  uint64_t address = args[0].As<BigInt>()->Uint64Value();
  size_t size = static_cast<size_t>(args[1].As<v8::Uint32>()->Value());

  if (size > 8) {
    if (read_buf_.size() < size) read_buf_.resize(size);
    if (!SafeMemcpy(read_buf_.data(), reinterpret_cast<void*>(address), size)) {
      isolate->ThrowError("Access violation reading memory");
      return;
    }

    uint32_t checksum = Crc32Hash(read_buf_.data(), size);
    MemCacheKey key{address, static_cast<uint32_t>(size)};
    auto [it, inserted] = checksum_cache_.emplace(key, checksum);
    if (!inserted) {
      if (it->second == checksum) {
        args.GetReturnValue().SetUndefined();
        return;
      }
      it->second = checksum;
    }

    auto backing_store = v8::ArrayBuffer::NewBackingStore(isolate, size);
    std::memcpy(backing_store->Data(), read_buf_.data(), size);
    Local<v8::ArrayBuffer> ab = v8::ArrayBuffer::New(isolate, std::move(backing_store));
    args.GetReturnValue().Set(Uint8Array::New(ab, 0, size));
  } else {
    auto backing_store = v8::ArrayBuffer::NewBackingStore(isolate, size);
    if (!SafeMemcpy(backing_store->Data(), reinterpret_cast<void*>(address), size)) {
      isolate->ThrowError("Access violation reading memory");
      return;
    }
    Local<v8::ArrayBuffer> ab = v8::ArrayBuffer::New(isolate, std::move(backing_store));
    args.GetReturnValue().Set(Uint8Array::New(ab, 0, size));
  }
}

// readMemoryIntoIfChanged(address: BigInt, buffer: Uint8Array) -> bool
static void ReadMemoryIntoIfChanged(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  uint64_t address = args[0].As<BigInt>()->Uint64Value();
  Local<Uint8Array> dst_arr = args[1].As<Uint8Array>();
  size_t size = dst_arr->ByteLength();
  void* dest = static_cast<uint8_t*>(dst_arr->Buffer()->Data()) + dst_arr->ByteOffset();

  if (size > 8) {
    if (read_buf_.size() < size) read_buf_.resize(size);
    if (!SafeMemcpy(read_buf_.data(), reinterpret_cast<void*>(address), size)) {
      isolate->ThrowError("Access violation reading memory");
      return;
    }
    std::memcpy(dest, read_buf_.data(), size);

    uint32_t checksum = Crc32Hash(read_buf_.data(), size);
    MemCacheKey key{address, static_cast<uint32_t>(size)};
    auto [it, inserted] = checksum_cache_.emplace(key, checksum);
    if (!inserted) {
      if (it->second == checksum) {
        args.GetReturnValue().Set(v8::False(isolate));
        return;
      }
      it->second = checksum;
    }
  } else {
    if (!SafeMemcpy(dest, reinterpret_cast<void*>(address), size)) {
      isolate->ThrowError("Access violation reading memory");
      return;
    }
  }
  args.GetReturnValue().Set(v8::True(isolate));
}

// clearChecksumCache() -> void
static void ClearChecksumCache(const FunctionCallbackInfo<Value>& args) {
  checksum_cache_.clear();
}

// readMemoryInto(address: BigInt, buffer: Uint8Array) -> void
static void ReadMemoryInto(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  CHECK(args[0]->IsBigInt());
  CHECK(args[1]->IsUint8Array());

  uint64_t address = args[0].As<BigInt>()->Uint64Value();
  Local<Uint8Array> buffer = args[1].As<Uint8Array>();

  size_t size = buffer->ByteLength();
  void* dest = static_cast<uint8_t*>(buffer->Buffer()->Data()) + buffer->ByteOffset();

  if (!SafeMemcpy(dest, reinterpret_cast<void*>(address), size)) {
    isolate->ThrowError("Access violation reading memory");
    return;
  }
}

// writeMemory(address: BigInt, data: Uint8Array) -> void
static void WriteMemory(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();

  uint64_t address = args[0]->IsBigInt() ? args[0].As<BigInt>()->Uint64Value()
                                         : static_cast<uint64_t>(args[0]->NumberValue(context).FromMaybe(0));

  Local<Uint8Array> data = args[1].As<Uint8Array>();
  size_t len = data->Length();
  void* src = static_cast<uint8_t*>(data->Buffer()->Data()) + data->ByteOffset();

  if (!SafeMemcpy(reinterpret_cast<void*>(address), src, len)) {
    isolate->ThrowError("Access violation writing memory");
    return;
  }
}

static std::vector<uint8_t*> allocated_buffers_;

// allocateTestMemory(size) -> BigInt (address)
static void AllocateTestMemory(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();

  size_t size = args[0]->Uint32Value(context).FromMaybe(0);
  if (size == 0) {
    THROW_ERR_INVALID_ARG_TYPE(isolate, "size must be > 0");
    return;
  }

  uint8_t* buffer = new uint8_t[size];
  std::memset(buffer, 0, size);
  allocated_buffers_.push_back(buffer);

  args.GetReturnValue().Set(BigInt::NewFromUnsigned(isolate, reinterpret_cast<uint64_t>(buffer)));
}

// freeTestMemory(address) -> void
static void FreeTestMemory(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();

  uint64_t address = args[0]->IsBigInt() ? args[0].As<BigInt>()->Uint64Value()
                                         : static_cast<uint64_t>(args[0]->NumberValue(context).FromMaybe(0));

  uint8_t* ptr = reinterpret_cast<uint8_t*>(address);

  auto it = std::find(allocated_buffers_.begin(), allocated_buffers_.end(), ptr);
  if (it != allocated_buffers_.end()) {
    delete[] *it;
    allocated_buffers_.erase(it);
  }
}

// freeAllTestMemory() -> void
static void FreeAllTestMemory(const FunctionCallbackInfo<Value>& args) {
  for (uint8_t* buffer : allocated_buffers_) {
    delete[] buffer;
  }
  allocated_buffers_.clear();
}

// highResolutionTime() -> BigInt (nanoseconds)
static void HighResolutionTime(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  auto now = std::chrono::high_resolution_clock::now();
  auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();

  args.GetReturnValue().Set(BigInt::New(isolate, ns));
}

static void AcquireGameLock(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(args);

  int timeout_ms = 100;
  if (args.Length() > 0 && args[0]->IsNumber()) {
    timeout_ms = static_cast<int>(args[0]->NumberValue(context).FromMaybe(100));
  }

  bool acquired = env->game_lock()->Acquire(std::chrono::milliseconds(timeout_ms));
  args.GetReturnValue().Set(v8::Boolean::New(isolate, acquired));
}

static void ReleaseGameLock(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  env->game_lock()->Release();
}

static void IsGameLockHeld(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(args);
  args.GetReturnValue().Set(v8::Boolean::New(isolate, env->game_lock()->IsHeld()));
}

static void IsGameLockOpen(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Environment* env = Environment::GetCurrent(args);
  args.GetReturnValue().Set(v8::Boolean::New(isolate, env->game_lock()->IsOpen()));
}

static void CreatePerIsolateProperties(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();

  SetMethod(isolate, target, "readMemory", ReadMemory);
  SetMethod(isolate, target, "readMemoryFast", ReadMemoryFast);
  SetMethod(isolate, target, "readMemoryIfChanged", ReadMemoryIfChanged);
  SetMethod(isolate, target, "readMemoryIntoIfChanged", ReadMemoryIntoIfChanged);
  SetMethod(isolate, target, "readMemoryInto", ReadMemoryInto);
  SetMethod(isolate, target, "writeMemory", WriteMemory);
  SetMethod(isolate, target, "clearChecksumCache", ClearChecksumCache);

  SetMethod(isolate, target, "allocateTestMemory", AllocateTestMemory);
  SetMethod(isolate, target, "freeTestMemory", FreeTestMemory);
  SetMethod(isolate, target, "freeAllTestMemory", FreeAllTestMemory);
  SetMethod(isolate, target, "highResolutionTime", HighResolutionTime);

  SetMethod(isolate, target, "acquireGameLock", AcquireGameLock);
  SetMethod(isolate, target, "releaseGameLock", ReleaseGameLock);
  SetMethod(isolate, target, "isGameLockHeld", IsGameLockHeld);
  SetMethod(isolate, target, "isGameLockOpen", IsGameLockOpen);
}

static void CreatePerContextProperties(Local<Object> target, Local<Context> context) {}

NYX_BINDING_PER_ISOLATE_INIT(memory, CreatePerIsolateProperties)
NYX_BINDING_CONTEXT_AWARE(memory, CreatePerContextProperties)

}  // namespace nyx
