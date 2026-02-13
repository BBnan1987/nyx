#include "nyx/isolate_data.h"
#include "nyx/nyx.h"
#include "nyx/nyx_binding.h"
#include "nyx/util.h"

namespace nyx {

using v8::Context;
using v8::FunctionCallbackInfo;
using v8::HandleScope;
using v8::Int32;
using v8::Isolate;
using v8::Local;
using v8::Message;
using v8::Object;
using v8::ObjectTemplate;
using v8::StackFrame;
using v8::StackTrace;
using v8::String;
using v8::Value;

// write(fd, message)
// fd: 1=stdout, 2=stderr
static void Write(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() < 2) return;

  int fd = args[0].As<Int32>()->Value();
  String::Utf8Value msg(isolate, args[1]);

  if (!*msg) return;

  FILE* stream = (fd == 2) ? GetStderr() : GetStdout();
  fprintf(stream, "%s\n", *msg);
  fflush(stream);
}

// getStackTrace() -> string
// Returns the current V8 stack trace formatted as a string.
static void GetStackTrace(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  HandleScope scope(isolate);

  Local<StackTrace> stack = StackTrace::CurrentStackTrace(isolate, 16, StackTrace::kDetailed);
  int frame_count = stack->GetFrameCount();

  std::string result;
  // Skip frame 0 (getStackTrace itself) and frame 1 (console.trace wrapper)
  for (int i = 2; i < frame_count; ++i) {
    Local<StackFrame> frame = stack->GetFrame(isolate, i);

    String::Utf8Value fn_name(isolate, frame->GetFunctionName());
    String::Utf8Value script_name(isolate, frame->GetScriptName());
    int line = frame->GetLineNumber();
    int col = frame->GetColumn();

    result += "    at ";
    if (*fn_name && fn_name.length() > 0) {
      result += *fn_name;
      result += " (";
    }
    if (*script_name) {
      result += *script_name;
    } else {
      result += "<anonymous>";
    }
    result += ":";
    result += std::to_string(line);
    result += ":";
    result += std::to_string(col);
    if (*fn_name && fn_name.length() > 0) {
      result += ")";
    }
    if (i < frame_count - 1) {
      result += "\n";
    }
  }

  args.GetReturnValue().Set(OneByteString(isolate, result));
}

static void CreatePerIsolateProperties(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  SetMethod(isolate, target, "write", Write);
  SetMethod(isolate, target, "getStackTrace", GetStackTrace);
}

static void CreatePerContextProperties(Local<Object> target, Local<v8::Context> context) {}

NYX_BINDING_PER_ISOLATE_INIT(console, CreatePerIsolateProperties)
NYX_BINDING_CONTEXT_AWARE(console, CreatePerContextProperties)

}  // namespace nyx
