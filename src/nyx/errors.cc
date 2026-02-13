#include "nyx/errors.h"

#include "nyx/util.h"

namespace nyx {

using v8::EscapableHandleScope;
using v8::Exception;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Value;

static Local<Value> CreateError(Isolate* isolate, const char* code, const char* message_format, const char* detail) {
  EscapableHandleScope scope(isolate);

  char message[1024];
  snprintf(message, sizeof(message), message_format, detail);

  Local<Value> error = Exception::Error(OneByteString(isolate, message));
  Local<Object> error_obj = error.As<Object>();

  Local<v8::Context> context = isolate->GetCurrentContext();
  error_obj->Set(context, OneByteString(isolate, "code"), OneByteString(isolate, code)).Check();

  return scope.Escape(error);
}

#define V(Code, Message)                                                                                               \
  Local<Value> Code(Isolate* isolate, const char* detail) {                                                            \
    return CreateError(isolate, #Code, Message, detail);                                                               \
  }
ERRORS(V)
#undef V

}  // namespace nyx
