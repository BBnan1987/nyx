#pragma once

#include <v8.h>
#include <string>

namespace nyx {

// Error definitions: V(Code, MessageGenerator)
// MessageGenerator is a function/lambda that takes args and returns std::string
#define ERRORS(V)                                                                                                      \
  V(ERR_CONSTRUCT_CALL_REQUIRED, "%s must be called with new")                                                         \
  V(ERR_INVALID_MODULE, "No such binding: %s")                                                                         \
  V(ERR_FILE_NOT_FOUND, "Cannot find file: %s")                                                                        \
  V(ERR_FILE_READ_FAILED, "Failed to read file: %s")                                                                   \
  V(ERR_INVALID_ARG_TYPE, "Invalid argument type: expected %s")                                                        \
  V(ERR_INVALID_STATE, "Invalid state: %s")                                                                            \
  V(ERR_MISSING_ARGS, "Missing required argument: %s")                                                                 \
  V(ERR_OUT_OF_RANGE, "Value out of range: %s")                                                                        \
  V(ERR_OPERATION_FAILED, "Operation failed: %s")                                                                      \
  V(ERR_SOURCE_PHASE_NOT_DEFINED, "Source phase not defined: %s")                                                      \
  V(ERR_MODULE_LINK_MISMATCH,                                                                                          \
    "Module request '%s' at index %d must be linked to the same module requested at index %d")                         \
  V(ERR_VM_MODULE_LINK_FAILURE, "request for '%s' is from invalid module")                                             \
  V(ERR_REQUIRE_ASYNC_MODULE, "unexpected top-level await require async module")                                       \
  V(ERR_MODULE_NOT_INSTANTIATED, "module is not instantiated")                                                         \
  V(ERR_EXECUTION_ENVIRONMENT_NOT_AVAILABLE, "execution environment not available")

#define V(Code, Message)                                                                                               \
  v8::Local<v8::Value> Code(v8::Isolate* isolate, const char* detail = "");                                            \
  inline void THROW_##Code(v8::Isolate* isolate, const char* detail = "") {                                            \
    isolate->ThrowException(Code(isolate, detail));                                                                    \
  }
ERRORS(V)
#undef V

#define V(Code, Message)                                                                                               \
  inline v8::Local<v8::Value> Code(v8::Isolate* isolate, const std::string& detail) {                                  \
    return Code(isolate, detail.c_str());                                                                              \
  }                                                                                                                    \
  inline void THROW_##Code(v8::Isolate* isolate, const std::string& detail) {                                          \
    THROW_##Code(isolate, detail.c_str());                                                                             \
  }
ERRORS(V)
#undef V

}  // namespace nyx
