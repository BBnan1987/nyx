#pragma once

#include <v8.h>
#include <ranges>
#include <set>
#include <string>

namespace nyx {

#define NYX_STRINGIFY_(x) #x
#define NYX_STRINGIFY(x) NYX_STRINGIFY_(x)

#define NYX_NOINLINE __declspec(noinline)

struct AssertionInfo {
  const char* file_line;  // filename:line
  const char* what;
  const char* where;
};

void Assert(const AssertionInfo& info);
// void DumpNativeBacktrace(FILE* fp);
// void DumpJavaScriptBacktrace(FILE* fp);

#define ABORT_NO_BACKTRACE()                                                                                           \
  __debugbreak();                                                                                                      \
  _exit(129);
#define ABORT()                                                                                                        \
  do {                                                                                                                 \
    /*::nyx::DumpNativeBacktrace(stderr);*/                                                                            \
    /*::nyx::DumpJavaScriptBacktrace(stderr);*/                                                                        \
    ABORT_NO_BACKTRACE();                                                                                              \
  } while (0)

#define ERROR_AND_ABORT(expr)                                                                                          \
  do {                                                                                                                 \
    static const ::nyx::AssertionInfo assert_info = {__FILE__ ":" NYX_STRINGIFY(__LINE__), #expr, __FUNCSIG__};        \
    ::nyx::Assert(assert_info);                                                                                        \
    ABORT_NO_BACKTRACE();                                                                                              \
  } while (0)

#if defined(_DEBUG)
#define CHECK(expr)                                                                                                    \
  do {                                                                                                                 \
    if (!(expr)) [[unlikely]] {                                                                                        \
      ERROR_AND_ABORT(expr);                                                                                           \
    }                                                                                                                  \
  } while (0)

#define CHECK_EQ(a, b) CHECK((a) == (b))
#define CHECK_GE(a, b) CHECK((a) >= (b))
#define CHECK_GT(a, b) CHECK((a) > (b))
#define CHECK_LE(a, b) CHECK((a) <= (b))
#define CHECK_LT(a, b) CHECK((a) < (b))
#define CHECK_NE(a, b) CHECK((a) != (b))
#define CHECK_NULL(val) CHECK((val) == nullptr)
#define CHECK_NOT_NULL(val) CHECK((val) != nullptr)
#define CHECK_IMPLIES(a, b) CHECK(!(a) || (b))
#else
#define CHECK(expr)
#define CHECK_EQ(a, b)
#define CHECK_GE(a, b)
#define CHECK_GT(a, b)
#define CHECK_LE(a, b)
#define CHECK_LT(a, b)
#define CHECK_NE(a, b)
#define CHECK_NULL(val)
#define CHECK_NOT_NULL(val)
#define CHECK_IMPLIES(a, b)
#endif

#define UNREACHABLE(...) ERROR_AND_ABORT("Unreachable code reached" __VA_OPT__(": ") __VA_ARGS__)

FILE* GetStderr();

static inline void ReportException(v8::Isolate* isolate, v8::TryCatch* try_catch) {
  v8::Local<v8::Value> exception = try_catch->Exception();
  v8::String::Utf8Value exception_str(isolate, exception);
  const char* msg = *exception_str ? *exception_str : "(stringify failed)";
  FILE* err = GetStderr();
  fprintf(err, "%s\n", msg);
  fflush(err);
}

class TryCatchScope : public v8::TryCatch {
 public:
  TryCatchScope(v8::Isolate* isolate) : v8::TryCatch(isolate), isolate_(isolate) {}
  ~TryCatchScope() {
    if (HasCaught() && !HasTerminated()) {
      ReportException(isolate_, this);
    }
  }

 private:
  v8::Isolate* isolate_;
};

static inline v8::Local<v8::String> OneByteString(v8::Isolate* isolate, std::string_view str) {
  return v8::String::NewFromUtf8(isolate, str.data(), v8::NewStringType::kNormal, str.length()).ToLocalChecked();
}

static inline v8::Local<v8::String> OneByteString(v8::Isolate* isolate, const char* str, size_t length) {
  return v8::String::NewFromUtf8(isolate, str, v8::NewStringType::kNormal, length).ToLocalChecked();
}

template <int N>
static inline v8::Local<v8::String> FixedOneByteString(v8::Isolate* isolate, const char (&data)[N]) {
  return OneByteString(isolate, data, N - 1);
}

template <std::size_t N>
static inline v8::Local<v8::String> FixedOneByteString(v8::Isolate* isolate, const std::array<char, N>& arr) {
  return OneByteString(isolate, arr.data(), N - 1);
}

template <typename T, size_t N>
constexpr size_t arraysize(const T (&)[N]) {
  return N;
}

template <typename T, size_t N>
constexpr size_t strsize(const T (&)[N]) {
  return N - 1;
}

inline v8::Maybe<void> FromV8Array(v8::Local<v8::Context> context,
                                   v8::Local<v8::Array> js_array,
                                   std::vector<v8::Global<v8::Value>>* out);

inline v8::MaybeLocal<v8::Value> ToV8Value(v8::Local<v8::Context> context,
                                           std::string_view str,
                                           v8::Isolate* isolate = nullptr);
template <typename T,
          typename test_for_number = typename std::enable_if<std::numeric_limits<T>::is_specialized, bool>::type>
inline v8::MaybeLocal<v8::Value> ToV8Value(v8::Local<v8::Context> context,
                                           const T& number,
                                           v8::Isolate* isolate = nullptr);
template <typename T>
inline v8::MaybeLocal<v8::Value> ToV8Value(v8::Local<v8::Context> context,
                                           const std::vector<T>& vec,
                                           v8::Isolate* isolate = nullptr);
template <typename T>
inline v8::MaybeLocal<v8::Value> ToV8Value(v8::Local<v8::Context> context,
                                           const std::set<T>& set,
                                           v8::Isolate* isolate = nullptr);
template <typename T, typename U>
inline v8::MaybeLocal<v8::Value> ToV8Value(v8::Local<v8::Context> context,
                                           const std::unordered_map<T, U>& map,
                                           v8::Isolate* isolate = nullptr);
template <typename T, std::size_t U>
inline v8::MaybeLocal<v8::Value> ToV8Value(v8::Local<v8::Context> context,
                                           const std::ranges::elements_view<T, U>& vec,
                                           v8::Isolate* isolate = nullptr);

// Allocates an array of member type T. For up to kStackStorageSize items,
// the stack is used, otherwise malloc().
template <typename T, size_t kStackStorageSize = 1024>
class MaybeStackBuffer {
 public:
  MaybeStackBuffer() : length_(0), capacity_(arraysize(buf_st_)), buf_(buf_st_) {
    // Default to a zero-length, null-terminated buffer.
    buf_[0] = T();
  }

  explicit MaybeStackBuffer(size_t storage) : MaybeStackBuffer() { AllocateSufficientStorage(storage); }

  ~MaybeStackBuffer() {
    if (IsAllocated()) free(buf_);
  }

  MaybeStackBuffer(const MaybeStackBuffer&) = delete;
  MaybeStackBuffer& operator=(const MaybeStackBuffer& other) = delete;

  const T* out() const { return buf_; }

  T* out() { return buf_; }

  T* operator*() { return buf_; }

  const T* operator*() const { return buf_; }

  T& operator[](size_t index) {
    CHECK_LT(index, length());
    return buf_[index];
  }

  const T& operator[](size_t index) const {
    CHECK_LT(index, length());
    return buf_[index];
  }

  size_t length() const { return length_; }

  // Current maximum capacity of the buffer with which SetLength() can be used
  // without first calling AllocateSufficientStorage().
  size_t capacity() const { return capacity_; }

  // Make sure enough space for `storage` entries is available.
  // This method can be called multiple times throughout the lifetime of the
  // buffer, but once this has been called Invalidate() cannot be used.
  // Content of the buffer in the range [0, length()) is preserved.
  void AllocateSufficientStorage(size_t storage) {
    CHECK(!IsInvalidated());
    if (storage > capacity()) {
      bool was_allocated = IsAllocated();
      T* allocated_ptr = was_allocated ? buf_ : nullptr;
      buf_ = static_cast<T*>(realloc(allocated_ptr, storage));
      capacity_ = storage;
      if (!was_allocated && length_ > 0) {
        memcpy(buf_, buf_st_, length_ * sizeof(buf_[0]));
      }
    }

    length_ = storage;
  }

  void SetLength(size_t length) {
    CHECK_LE(length, capacity());
    length_ = length;
  }

  void SetLengthAndZeroTerminate(size_t length) {
    CHECK_LE(length + 1, capacity());
    SetLength(length);
    buf_[length] = T();
  }

  // Make dereferencing this object return nullptr.
  // This method can be called multiple times throughout the lifetime of the
  // buffer, but once this has been called AllocateSufficientStorage() cannot
  // be used.
  void Invalidate() {
    CHECK(!IsAllocated());
    capacity_ = 0;
    length_ = 0;
    buf_ = nullptr;
  }

  bool IsAllocated() const { return !IsInvalidated() && buf_ != buf_st_; }

  bool IsInvalidated() const { return buf_ == nullptr; }

  // Release ownership of the malloc'd buffer.
  // Note: This does not free the buffer.
  void Release() {
    CHECK(IsAllocated());
    buf_ = buf_st_;
    length_ = 0;
    capacity_ = arraysize(buf_st_);
  }

  inline std::basic_string<T> ToString() const { return {out(), length()}; }
  inline std::basic_string_view<T> ToStringView() const { return {out(), length()}; }

 private:
  size_t length_;
  size_t capacity_;
  T* buf_;
  T buf_st_[kStackStorageSize];
};

class Utf8Value : public MaybeStackBuffer<char> {
 public:
  explicit Utf8Value(v8::Isolate* isolate, v8::Local<v8::Value> value);

  inline std::string ToString() const { return std::string(out(), length()); }
  inline std::string_view ToStringView() const { return std::string_view(out(), length()); }

  inline bool operator==(const char* a) const { return strcmp(out(), a) == 0; }
  inline bool operator!=(const char* a) const { return !(*this == a); }
};

class TwoByteValue : public MaybeStackBuffer<uint16_t> {
 public:
  explicit TwoByteValue(v8::Isolate* isolate, v8::Local<v8::Value> value);
};

class BufferValue : public MaybeStackBuffer<char> {
 public:
  explicit BufferValue(v8::Isolate* isolate, v8::Local<v8::Value> value);

  inline std::string ToString() const { return std::string(out(), length()); }
  inline std::string_view ToStringView() const { return std::string_view(out(), length()); }
  inline std::u8string_view ToU8StringView() const {
    return std::u8string_view(reinterpret_cast<const char8_t*>(out()), length());
  }
};

static inline v8::Local<v8::FunctionTemplate> NewFunctionTemplate(
    v8::Isolate* isolate,
    v8::FunctionCallback callback,
    v8::Local<v8::Signature> signature = v8::Local<v8::Signature>(),
    v8::ConstructorBehavior behavior = v8::ConstructorBehavior::kAllow,
    v8::SideEffectType side_effect = v8::SideEffectType::kHasSideEffect,
    const v8::CFunction* c_function = nullptr) {
  return v8::FunctionTemplate::New(
      isolate, callback, v8::Local<v8::Value>(), signature, 0, behavior, side_effect, c_function);
}

static inline void SetMethod(v8::Isolate* isolate,
                             v8::Local<v8::ObjectTemplate> target,
                             const char* name,
                             v8::FunctionCallback callback) {
  target->Set(OneByteString(isolate, name), v8::FunctionTemplate::New(isolate, callback));
}

static inline void SetProperty(v8::Isolate* isolate,
                               v8::Local<v8::ObjectTemplate> target,
                               const char* name,
                               v8::FunctionCallback getter,
                               v8::FunctionCallback setter = nullptr) {
  v8::Local<v8::FunctionTemplate> t_getter;
  v8::Local<v8::FunctionTemplate> t_setter;
  if (getter) {
    t_getter = v8::FunctionTemplate::New(isolate, getter);
  }
  if (setter) {
    t_setter = v8::FunctionTemplate::New(isolate, setter);
  }
  target->SetAccessorProperty(OneByteString(isolate, name), t_getter, t_setter);
}

static inline void SetProtoProperty(v8::Isolate* isolate,
                                    v8::Local<v8::FunctionTemplate> target,
                                    const char* name,
                                    v8::FunctionCallback getter,
                                    v8::FunctionCallback setter = nullptr) {
  v8::Local<v8::FunctionTemplate> t_getter;
  v8::Local<v8::FunctionTemplate> t_setter;
  if (getter) {
    t_getter = v8::FunctionTemplate::New(isolate, getter);
  }
  if (setter) {
    t_setter = v8::FunctionTemplate::New(isolate, setter);
  }
  target->PrototypeTemplate()->SetAccessorProperty(OneByteString(isolate, name), t_getter, t_setter);
}

static inline void SetProtoMethod(v8::Isolate* isolate,
                                  v8::Local<v8::FunctionTemplate> target,
                                  const char* name,
                                  v8::FunctionCallback callback) {
  v8::Local<v8::FunctionTemplate> method = v8::FunctionTemplate::New(isolate, callback);
  target->PrototypeTemplate()->Set(OneByteString(isolate, name), method);
}

}  // namespace nyx

#include "util-inl.h"
