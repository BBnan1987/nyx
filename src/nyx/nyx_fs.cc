#include <uv.h>

#include <fstream>
#include <sstream>

#include "nyx/env.h"
#include "nyx/errors.h"
#include "nyx/util.h"

namespace nyx {

using v8::Array;
using v8::ArrayBuffer;
using v8::Boolean;
using v8::Context;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::HandleScope;
using v8::Integer;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::ObjectTemplate;
using v8::Promise;
using v8::String;
using v8::Uint8Array;
using v8::Value;

class FsReqWrap {
 public:
  FsReqWrap(Environment* env, v8::Local<v8::Promise::Resolver> resolver) : env_(env) {
    resolver_.Reset(env->isolate(), resolver);
    req_.data = this;
  }

  ~FsReqWrap() { resolver_.Reset(); }

  uv_fs_t* req() { return &req_; }
  Environment* env() const { return env_; }
  v8::Local<v8::Promise::Resolver> resolver() const { return resolver_.Get(env_->isolate()); }

  void Resolve(v8::Local<v8::Value> value) {
    Isolate* isolate = env_->isolate();
    HandleScope handle_scope(isolate);
    Local<Context> context = env_->context();
    Context::Scope context_scope(context);

    Local<Promise::Resolver> res = resolver();
    res->Resolve(context, value).Check();
  }

  void Reject(const char* message) {
    Isolate* isolate = env_->isolate();
    HandleScope handle_scope(isolate);
    Local<Context> context = env_->context();
    Context::Scope context_scope(context);

    Local<String> error_msg = OneByteString(isolate, message);
    Local<Value> error = v8::Exception::Error(error_msg);
    Reject(error);
  }

  void Reject(v8::Local<v8::Value> error) {
    Isolate* isolate = env_->isolate();
    HandleScope handle_scope(isolate);
    Local<Context> context = env_->context();
    Context::Scope context_scope(context);

    Local<Promise::Resolver> res = resolver();
    res->Reject(context, error).Check();
  }

  void Cleanup() {
    uv_fs_req_cleanup(&req_);
    delete this;
  }

 private:
  Environment* env_;
  uv_fs_t req_;
  v8::Global<v8::Promise::Resolver> resolver_;
};

// readFileSync(path, encoding?)
static void ReadFileSync(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  HandleScope handle_scope(isolate);
  Local<Context> context = isolate->GetCurrentContext();

  if (args.Length() < 1 || !args[0]->IsString()) {
    THROW_ERR_INVALID_ARG_TYPE(isolate, "path must be a string");
    return;
  }

  String::Utf8Value path(isolate, args[0]);

  std::ifstream file(*path, std::ios::binary);
  if (!file.good()) {
    THROW_ERR_FILE_NOT_FOUND(isolate, *path);
    return;
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string content = buffer.str();

  bool return_buffer = true;
  if (args.Length() > 1) {
    if (args[1]->IsString()) {
      String::Utf8Value encoding(isolate, args[1]);
      if (strcmp(*encoding, "utf8") == 0 || strcmp(*encoding, "utf-8") == 0) {
        return_buffer = false;
      }
    } else if (args[1]->IsObject()) {
      Local<Object> options = args[1].As<Object>();
      Local<Value> encoding_val;
      if (options->Get(context, OneByteString(isolate, "encoding")).ToLocal(&encoding_val) &&
          encoding_val->IsString()) {
        String::Utf8Value encoding(isolate, encoding_val);
        if (strcmp(*encoding, "utf8") == 0 || strcmp(*encoding, "utf-8") == 0) {
          return_buffer = false;
        }
      }
    }
  }

  if (return_buffer) {
    auto backing_store = v8::ArrayBuffer::NewBackingStore(isolate, content.size());
    memcpy(backing_store->Data(), content.data(), content.size());
    Local<v8::ArrayBuffer> array_buffer = v8::ArrayBuffer::New(isolate, std::move(backing_store));
    Local<v8::Uint8Array> uint8_array = v8::Uint8Array::New(array_buffer, 0, content.size());
    args.GetReturnValue().Set(uint8_array);
  } else {
    Local<String> result;
    if (!String::NewFromUtf8(isolate, content.c_str(), v8::NewStringType::kNormal, static_cast<int>(content.size()))
             .ToLocal(&result)) {
      return;
    }
    args.GetReturnValue().Set(result);
  }
}

// writeFileSync(path, data, encoding?)
static void WriteFileSync(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();

  if (args.Length() < 2) {
    THROW_ERR_MISSING_ARGS(isolate, "path and data");
    return;
  }

  if (!args[0]->IsString()) {
    THROW_ERR_INVALID_ARG_TYPE(isolate, "path must be a string");
    return;
  }

  String::Utf8Value path(isolate, args[0]);
  std::string content;

  if (args[1]->IsString()) {
    String::Utf8Value data(isolate, args[1]);
    content = std::string(*data, data.length());
  } else if (args[1]->IsUint8Array()) {
    Local<v8::Uint8Array> uint8_array = args[1].As<v8::Uint8Array>();
    content.resize(uint8_array->Length());
    uint8_array->CopyContents(content.data(), content.size());
  } else if (args[1]->IsArrayBuffer()) {
    Local<v8::ArrayBuffer> array_buffer = args[1].As<v8::ArrayBuffer>();
    content = std::string(static_cast<char*>(array_buffer->Data()), array_buffer->ByteLength());
  } else {
    THROW_ERR_INVALID_ARG_TYPE(isolate, "data must be a string or buffer");
    return;
  }

  std::ofstream file(*path, std::ios::binary);
  if (!file.good()) {
    THROW_ERR_OPERATION_FAILED(isolate, "Failed to open file for writing");
    return;
  }

  file.write(content.data(), content.size());
  if (!file.good()) {
    THROW_ERR_OPERATION_FAILED(isolate, "Failed to write to file");
    return;
  }

  args.GetReturnValue().SetUndefined();
}

// existsSync(path)
static void ExistsSync(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() < 1 || !args[0]->IsString()) {
    args.GetReturnValue().Set(false);
    return;
  }

  String::Utf8Value path(isolate, args[0]);

  uv_fs_t req;
  int result = uv_fs_stat(nullptr, &req, *path, nullptr);
  uv_fs_req_cleanup(&req);

  args.GetReturnValue().Set(result == 0);
}

// statSync(path)
static void StatSync(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();

  if (args.Length() < 1 || !args[0]->IsString()) {
    THROW_ERR_INVALID_ARG_TYPE(isolate, "path must be a string");
    return;
  }

  String::Utf8Value path(isolate, args[0]);

  uv_fs_t req;
  int result = uv_fs_stat(nullptr, &req, *path, nullptr);
  if (result != 0) {
    uv_fs_req_cleanup(&req);
    THROW_ERR_FILE_NOT_FOUND(isolate, *path);
    return;
  }

  const uv_stat_t* stat = &req.statbuf;

  Local<Object> stats = Object::New(isolate);
  stats->Set(context, OneByteString(isolate, "size"), Number::New(isolate, static_cast<double>(stat->st_size))).Check();
  stats->Set(context, OneByteString(isolate, "mode"), Integer::New(isolate, stat->st_mode)).Check();
  stats
      ->Set(context,
            OneByteString(isolate, "mtime"),
            Number::New(isolate, static_cast<double>(stat->st_mtim.tv_sec) * 1000))
      .Check();
  stats
      ->Set(context,
            OneByteString(isolate, "atime"),
            Number::New(isolate, static_cast<double>(stat->st_atim.tv_sec) * 1000))
      .Check();
  stats
      ->Set(context,
            OneByteString(isolate, "ctime"),
            Number::New(isolate, static_cast<double>(stat->st_ctim.tv_sec) * 1000))
      .Check();

  bool is_file = (stat->st_mode & S_IFMT) == S_IFREG;
  bool is_dir = (stat->st_mode & S_IFMT) == S_IFDIR;

  stats->Set(context, OneByteString(isolate, "isFile"), Boolean::New(isolate, is_file)).Check();
  stats->Set(context, OneByteString(isolate, "isDirectory"), Boolean::New(isolate, is_dir)).Check();

  uv_fs_req_cleanup(&req);
  args.GetReturnValue().Set(stats);
}

// readdirSync(path)
static void ReaddirSync(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();

  if (args.Length() < 1 || !args[0]->IsString()) {
    THROW_ERR_INVALID_ARG_TYPE(isolate, "path must be a string");
    return;
  }

  String::Utf8Value path(isolate, args[0]);

  uv_fs_t req;
  int result = uv_fs_scandir(nullptr, &req, *path, 0, nullptr);
  if (result < 0) {
    uv_fs_req_cleanup(&req);
    THROW_ERR_FILE_NOT_FOUND(isolate, *path);
    return;
  }

  Local<Array> entries = Array::New(isolate);
  uv_dirent_t entry;
  uint32_t index = 0;

  while (uv_fs_scandir_next(&req, &entry) != UV_EOF) {
    Local<String> name = OneByteString(isolate, entry.name);
    entries->Set(context, index++, name).Check();
  }

  uv_fs_req_cleanup(&req);
  args.GetReturnValue().Set(entries);
}

// mkdirSync(path, options?)
static void MkdirSync(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();

  if (args.Length() < 1 || !args[0]->IsString()) {
    THROW_ERR_INVALID_ARG_TYPE(isolate, "path must be a string");
    return;
  }

  String::Utf8Value path(isolate, args[0]);
  int mode = 0777;
  bool recursive = false;

  if (args.Length() > 1 && args[1]->IsObject()) {
    Local<Object> options = args[1].As<Object>();
    Local<Value> mode_val;
    if (options->Get(context, OneByteString(isolate, "mode")).ToLocal(&mode_val) && mode_val->IsNumber()) {
      mode = mode_val->Int32Value(context).FromMaybe(0777);
    }
    Local<Value> recursive_val;
    if (options->Get(context, OneByteString(isolate, "recursive")).ToLocal(&recursive_val)) {
      recursive = recursive_val->BooleanValue(isolate);
    }
  }

  uv_fs_t req;
  int result = uv_fs_mkdir(nullptr, &req, *path, mode, nullptr);
  uv_fs_req_cleanup(&req);

  if (result != 0 && result != UV_EEXIST) {
    if (result == UV_ENOENT && recursive) {
      // TODO: Implement recursive mkdir
      THROW_ERR_OPERATION_FAILED(isolate, "Recursive mkdir not yet implemented");
      return;
    }
    THROW_ERR_OPERATION_FAILED(isolate, uv_strerror(result));
    return;
  }

  args.GetReturnValue().SetUndefined();
}

// unlinkSync(path)
static void UnlinkSync(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() < 1 || !args[0]->IsString()) {
    THROW_ERR_INVALID_ARG_TYPE(isolate, "path must be a string");
    return;
  }

  String::Utf8Value path(isolate, args[0]);

  uv_fs_t req;
  int result = uv_fs_unlink(nullptr, &req, *path, nullptr);
  uv_fs_req_cleanup(&req);

  if (result != 0) {
    THROW_ERR_OPERATION_FAILED(isolate, uv_strerror(result));
    return;
  }

  args.GetReturnValue().SetUndefined();
}

// rmdirSync(path)
static void RmdirSync(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() < 1 || !args[0]->IsString()) {
    THROW_ERR_INVALID_ARG_TYPE(isolate, "path must be a string");
    return;
  }

  String::Utf8Value path(isolate, args[0]);

  uv_fs_t req;
  int result = uv_fs_rmdir(nullptr, &req, *path, nullptr);
  uv_fs_req_cleanup(&req);

  if (result != 0) {
    THROW_ERR_OPERATION_FAILED(isolate, uv_strerror(result));
    return;
  }

  args.GetReturnValue().SetUndefined();
}

// renameSync(oldPath, newPath)
static void RenameSync(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() < 2 || !args[0]->IsString() || !args[1]->IsString()) {
    THROW_ERR_INVALID_ARG_TYPE(isolate, "paths must be strings");
    return;
  }

  String::Utf8Value old_path(isolate, args[0]);
  String::Utf8Value new_path(isolate, args[1]);

  uv_fs_t req;
  int result = uv_fs_rename(nullptr, &req, *old_path, *new_path, nullptr);
  uv_fs_req_cleanup(&req);

  if (result != 0) {
    THROW_ERR_OPERATION_FAILED(isolate, uv_strerror(result));
    return;
  }

  args.GetReturnValue().SetUndefined();
}

// realpathSync(path)
static void RealpathSync(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() < 1 || !args[0]->IsString()) {
    THROW_ERR_INVALID_ARG_TYPE(isolate, "path must be a string");
    return;
  }

  String::Utf8Value path(isolate, args[0]);

  uv_fs_t req;
  int result = uv_fs_realpath(nullptr, &req, *path, nullptr);
  if (result != 0) {
    uv_fs_req_cleanup(&req);
    THROW_ERR_FILE_NOT_FOUND(isolate, *path);
    return;
  }

  Local<String> resolved = OneByteString(isolate, static_cast<char*>(req.ptr));
  uv_fs_req_cleanup(&req);
  args.GetReturnValue().Set(resolved);
}

// helper to build stats object from uv_stat_t
static Local<Object> BuildStatsObject(Isolate* isolate, Local<Context> context, const uv_stat_t* stat) {
  Local<Object> stats = Object::New(isolate);
  stats->Set(context, OneByteString(isolate, "size"), Number::New(isolate, static_cast<double>(stat->st_size))).Check();
  stats->Set(context, OneByteString(isolate, "mode"), Integer::New(isolate, stat->st_mode)).Check();
  stats
      ->Set(context,
            OneByteString(isolate, "mtime"),
            Number::New(isolate, static_cast<double>(stat->st_mtim.tv_sec) * 1000))
      .Check();
  stats
      ->Set(context,
            OneByteString(isolate, "atime"),
            Number::New(isolate, static_cast<double>(stat->st_atim.tv_sec) * 1000))
      .Check();
  stats
      ->Set(context,
            OneByteString(isolate, "ctime"),
            Number::New(isolate, static_cast<double>(stat->st_ctim.tv_sec) * 1000))
      .Check();

  bool is_file = (stat->st_mode & S_IFMT) == S_IFREG;
  bool is_dir = (stat->st_mode & S_IFMT) == S_IFDIR;

  stats->Set(context, OneByteString(isolate, "isFile"), Boolean::New(isolate, is_file)).Check();
  stats->Set(context, OneByteString(isolate, "isDirectory"), Boolean::New(isolate, is_dir)).Check();

  return stats;
}

static void StatCallback(uv_fs_t* req) {
  FsReqWrap* wrap = static_cast<FsReqWrap*>(req->data);
  Environment* env = wrap->env();
  Isolate* isolate = env->isolate();
  HandleScope handle_scope(isolate);
  Local<Context> context = env->context();

  if (req->result < 0) {
    wrap->Reject(uv_strerror(static_cast<int>(req->result)));
  } else {
    Local<Object> stats = BuildStatsObject(isolate, context, &req->statbuf);
    wrap->Resolve(stats);
  }

  wrap->Cleanup();
}

// stat(path) -> Promise<Stats>
static void Stat(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);

  if (args.Length() < 1 || !args[0]->IsString()) {
    THROW_ERR_INVALID_ARG_TYPE(isolate, "path must be a string");
    return;
  }

  String::Utf8Value path(isolate, args[0]);

  Local<Promise::Resolver> resolver = Promise::Resolver::New(context).ToLocalChecked();
  args.GetReturnValue().Set(resolver->GetPromise());

  FsReqWrap* wrap = new FsReqWrap(env, resolver);

  int result = uv_fs_stat(env->event_loop(), wrap->req(), *path, StatCallback);
  if (result < 0) {
    wrap->Reject(uv_strerror(result));
    wrap->Cleanup();
  }
}

static void ReaddirCallback(uv_fs_t* req) {
  FsReqWrap* wrap = static_cast<FsReqWrap*>(req->data);
  Environment* env = wrap->env();
  Isolate* isolate = env->isolate();
  HandleScope handle_scope(isolate);
  Local<Context> context = env->context();

  if (req->result < 0) {
    wrap->Reject(uv_strerror(static_cast<int>(req->result)));
  } else {
    Local<Array> entries = Array::New(isolate);
    uv_dirent_t entry;
    uint32_t index = 0;

    while (uv_fs_scandir_next(req, &entry) != UV_EOF) {
      Local<String> name = OneByteString(isolate, entry.name);
      entries->Set(context, index++, name).Check();
    }

    wrap->Resolve(entries);
  }

  wrap->Cleanup();
}

// readdir(path) -> Promise<string[]>
static void Readdir(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);

  if (args.Length() < 1 || !args[0]->IsString()) {
    THROW_ERR_INVALID_ARG_TYPE(isolate, "path must be a string");
    return;
  }

  String::Utf8Value path(isolate, args[0]);

  Local<Promise::Resolver> resolver = Promise::Resolver::New(context).ToLocalChecked();
  args.GetReturnValue().Set(resolver->GetPromise());

  FsReqWrap* wrap = new FsReqWrap(env, resolver);

  int result = uv_fs_scandir(env->event_loop(), wrap->req(), *path, 0, ReaddirCallback);
  if (result < 0) {
    wrap->Reject(uv_strerror(result));
    wrap->Cleanup();
  }
}

struct ReadFileReq {
  FsReqWrap* wrap;
  uv_file fd;
  char* buffer;
  size_t size;
  bool return_string;

  ReadFileReq(FsReqWrap* w) : wrap(w), fd(-1), buffer(nullptr), size(0), return_string(false) {}
  ~ReadFileReq() { delete[] buffer; }
};

static void ReadFileCloseCallback(uv_fs_t* req);
static void ReadFileReadCallback(uv_fs_t* req);
static void ReadFileFstatCallback(uv_fs_t* req);
static void ReadFileOpenCallback(uv_fs_t* req);

static void ReadFileCloseCallback(uv_fs_t* req) {
  ReadFileReq* rf_req = static_cast<ReadFileReq*>(req->data);
  FsReqWrap* wrap = rf_req->wrap;
  Environment* env = wrap->env();
  Isolate* isolate = env->isolate();
  HandleScope handle_scope(isolate);
  Local<Context> context = env->context();

  if (rf_req->return_string) {
    Local<String> result;
    if (String::NewFromUtf8(isolate, rf_req->buffer, v8::NewStringType::kNormal, static_cast<int>(rf_req->size))
            .ToLocal(&result)) {
      wrap->Resolve(result);
    } else {
      wrap->Reject("Failed to create string from file contents");
    }
  } else {
    auto backing_store = ArrayBuffer::NewBackingStore(isolate, rf_req->size);
    memcpy(backing_store->Data(), rf_req->buffer, rf_req->size);
    Local<ArrayBuffer> array_buffer = ArrayBuffer::New(isolate, std::move(backing_store));
    Local<Uint8Array> uint8_array = Uint8Array::New(array_buffer, 0, rf_req->size);
    wrap->Resolve(uint8_array);
  }

  uv_fs_req_cleanup(req);
  delete rf_req;
  wrap->Cleanup();
}

static void ReadFileReadCallback(uv_fs_t* req) {
  ReadFileReq* rf_req = static_cast<ReadFileReq*>(req->data);
  FsReqWrap* wrap = rf_req->wrap;
  Environment* env = wrap->env();

  if (req->result < 0) {
    wrap->Reject(uv_strerror(static_cast<int>(req->result)));
    uv_fs_req_cleanup(req);
    delete rf_req;
    wrap->Cleanup();
    return;
  }

  uv_fs_req_cleanup(req);
  int result = uv_fs_close(env->event_loop(), wrap->req(), rf_req->fd, ReadFileCloseCallback);
  wrap->req()->data = rf_req;

  if (result < 0) {
    wrap->Reject(uv_strerror(result));
    delete rf_req;
    wrap->Cleanup();
  }
}

static void ReadFileFstatCallback(uv_fs_t* req) {
  ReadFileReq* rf_req = static_cast<ReadFileReq*>(req->data);
  FsReqWrap* wrap = rf_req->wrap;
  Environment* env = wrap->env();

  if (req->result < 0) {
    wrap->Reject(uv_strerror(static_cast<int>(req->result)));
    uv_fs_req_cleanup(req);
    delete rf_req;
    wrap->Cleanup();
    return;
  }

  rf_req->size = static_cast<size_t>(req->statbuf.st_size);
  rf_req->buffer = new char[rf_req->size];

  uv_buf_t buf = uv_buf_init(rf_req->buffer, static_cast<unsigned int>(rf_req->size));

  uv_fs_req_cleanup(req);
  int result = uv_fs_read(env->event_loop(), wrap->req(), rf_req->fd, &buf, 1, 0, ReadFileReadCallback);
  wrap->req()->data = rf_req;

  if (result < 0) {
    wrap->Reject(uv_strerror(result));
    delete rf_req;
    wrap->Cleanup();
  }
}

static void ReadFileOpenCallback(uv_fs_t* req) {
  ReadFileReq* rf_req = static_cast<ReadFileReq*>(req->data);
  FsReqWrap* wrap = rf_req->wrap;
  Environment* env = wrap->env();

  if (req->result < 0) {
    wrap->Reject(uv_strerror(static_cast<int>(req->result)));
    uv_fs_req_cleanup(req);
    delete rf_req;
    wrap->Cleanup();
    return;
  }

  rf_req->fd = static_cast<uv_file>(req->result);

  uv_fs_req_cleanup(req);
  int result = uv_fs_fstat(env->event_loop(), wrap->req(), rf_req->fd, ReadFileFstatCallback);
  wrap->req()->data = rf_req;

  if (result < 0) {
    wrap->Reject(uv_strerror(result));
    delete rf_req;
    wrap->Cleanup();
  }
}

// readFile(path, encoding?) -> Promise<string|Uint8Array>
static void ReadFile(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);

  if (args.Length() < 1 || !args[0]->IsString()) {
    THROW_ERR_INVALID_ARG_TYPE(isolate, "path must be a string");
    return;
  }

  String::Utf8Value path(isolate, args[0]);

  bool return_string = false;
  if (args.Length() > 1) {
    if (args[1]->IsString()) {
      String::Utf8Value encoding(isolate, args[1]);
      if (strcmp(*encoding, "utf8") == 0 || strcmp(*encoding, "utf-8") == 0) {
        return_string = true;
      }
    } else if (args[1]->IsObject()) {
      Local<Object> options = args[1].As<Object>();
      Local<Value> encoding_val;
      if (options->Get(context, OneByteString(isolate, "encoding")).ToLocal(&encoding_val) &&
          encoding_val->IsString()) {
        String::Utf8Value encoding(isolate, encoding_val);
        if (strcmp(*encoding, "utf8") == 0 || strcmp(*encoding, "utf-8") == 0) {
          return_string = true;
        }
      }
    }
  }

  Local<Promise::Resolver> resolver = Promise::Resolver::New(context).ToLocalChecked();
  args.GetReturnValue().Set(resolver->GetPromise());

  FsReqWrap* wrap = new FsReqWrap(env, resolver);
  ReadFileReq* rf_req = new ReadFileReq(wrap);
  rf_req->return_string = return_string;

  int result = uv_fs_open(env->event_loop(), wrap->req(), *path, O_RDONLY, 0, ReadFileOpenCallback);
  wrap->req()->data = rf_req;

  if (result < 0) {
    wrap->Reject(uv_strerror(result));
    delete rf_req;
    wrap->Cleanup();
  }
}

struct WriteFileReq {
  FsReqWrap* wrap;
  uv_file fd;
  char* buffer;
  size_t size;

  WriteFileReq(FsReqWrap* w) : wrap(w), fd(-1), buffer(nullptr), size(0) {}
  ~WriteFileReq() { delete[] buffer; }
};

static void WriteFileCloseCallback(uv_fs_t* req);
static void WriteFileWriteCallback(uv_fs_t* req);
static void WriteFileOpenCallback(uv_fs_t* req);

static void WriteFileCloseCallback(uv_fs_t* req) {
  WriteFileReq* wf_req = static_cast<WriteFileReq*>(req->data);
  FsReqWrap* wrap = wf_req->wrap;

  uv_fs_req_cleanup(req);
  wrap->Resolve(v8::Undefined(wrap->env()->isolate()));
  delete wf_req;
  wrap->Cleanup();
}

static void WriteFileWriteCallback(uv_fs_t* req) {
  WriteFileReq* wf_req = static_cast<WriteFileReq*>(req->data);
  FsReqWrap* wrap = wf_req->wrap;
  Environment* env = wrap->env();

  if (req->result < 0) {
    wrap->Reject(uv_strerror(static_cast<int>(req->result)));
    uv_fs_req_cleanup(req);
    delete wf_req;
    wrap->Cleanup();
    return;
  }

  // Close the file
  uv_fs_req_cleanup(req);
  int result = uv_fs_close(env->event_loop(), wrap->req(), wf_req->fd, WriteFileCloseCallback);
  wrap->req()->data = wf_req;

  if (result < 0) {
    wrap->Reject(uv_strerror(result));
    delete wf_req;
    wrap->Cleanup();
  }
}

static void WriteFileOpenCallback(uv_fs_t* req) {
  WriteFileReq* wf_req = static_cast<WriteFileReq*>(req->data);
  FsReqWrap* wrap = wf_req->wrap;
  Environment* env = wrap->env();

  if (req->result < 0) {
    wrap->Reject(uv_strerror(static_cast<int>(req->result)));
    uv_fs_req_cleanup(req);
    delete wf_req;
    wrap->Cleanup();
    return;
  }

  wf_req->fd = static_cast<uv_file>(req->result);

  uv_buf_t buf = uv_buf_init(wf_req->buffer, static_cast<unsigned int>(wf_req->size));

  uv_fs_req_cleanup(req);
  int result = uv_fs_write(env->event_loop(), wrap->req(), wf_req->fd, &buf, 1, 0, WriteFileWriteCallback);
  wrap->req()->data = wf_req;

  if (result < 0) {
    wrap->Reject(uv_strerror(result));
    delete wf_req;
    wrap->Cleanup();
  }
}

// writeFile(path, data) -> Promise<void>
static void WriteFile(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);

  if (args.Length() < 2) {
    THROW_ERR_MISSING_ARGS(isolate, "path and data");
    return;
  }

  if (!args[0]->IsString()) {
    THROW_ERR_INVALID_ARG_TYPE(isolate, "path must be a string");
    return;
  }

  String::Utf8Value path(isolate, args[0]);

  char* buffer = nullptr;
  size_t size = 0;

  if (args[1]->IsString()) {
    String::Utf8Value data(isolate, args[1]);
    size = data.length();
    buffer = new char[size];
    memcpy(buffer, *data, size);
  } else if (args[1]->IsUint8Array()) {
    Local<Uint8Array> uint8_array = args[1].As<Uint8Array>();
    size = uint8_array->Length();
    buffer = new char[size];
    uint8_array->CopyContents(buffer, size);
  } else if (args[1]->IsArrayBuffer()) {
    Local<ArrayBuffer> array_buffer = args[1].As<ArrayBuffer>();
    size = array_buffer->ByteLength();
    buffer = new char[size];
    memcpy(buffer, array_buffer->Data(), size);
  } else {
    THROW_ERR_INVALID_ARG_TYPE(isolate, "data must be a string or buffer");
    return;
  }

  Local<Promise::Resolver> resolver = Promise::Resolver::New(context).ToLocalChecked();
  args.GetReturnValue().Set(resolver->GetPromise());

  FsReqWrap* wrap = new FsReqWrap(env, resolver);
  WriteFileReq* wf_req = new WriteFileReq(wrap);
  wf_req->buffer = buffer;
  wf_req->size = size;

  int flags = O_WRONLY | O_CREAT | O_TRUNC;
  int result = uv_fs_open(env->event_loop(), wrap->req(), *path, flags, 0666, WriteFileOpenCallback);
  wrap->req()->data = wf_req;

  if (result < 0) {
    wrap->Reject(uv_strerror(result));
    delete wf_req;
    wrap->Cleanup();
  }
}

static void MkdirCallback(uv_fs_t* req) {
  FsReqWrap* wrap = static_cast<FsReqWrap*>(req->data);

  if (req->result < 0 && req->result != UV_EEXIST) {
    wrap->Reject(uv_strerror(static_cast<int>(req->result)));
  } else {
    wrap->Resolve(v8::Undefined(wrap->env()->isolate()));
  }

  wrap->Cleanup();
}

// mkdir(path, options?) -> Promise<void>
static void Mkdir(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);

  if (args.Length() < 1 || !args[0]->IsString()) {
    THROW_ERR_INVALID_ARG_TYPE(isolate, "path must be a string");
    return;
  }

  String::Utf8Value path(isolate, args[0]);
  int mode = 0777;

  if (args.Length() > 1 && args[1]->IsObject()) {
    Local<Object> options = args[1].As<Object>();
    Local<Value> mode_val;
    if (options->Get(context, OneByteString(isolate, "mode")).ToLocal(&mode_val) && mode_val->IsNumber()) {
      mode = mode_val->Int32Value(context).FromMaybe(0777);
    }
  }

  Local<Promise::Resolver> resolver = Promise::Resolver::New(context).ToLocalChecked();
  args.GetReturnValue().Set(resolver->GetPromise());

  FsReqWrap* wrap = new FsReqWrap(env, resolver);

  int result = uv_fs_mkdir(env->event_loop(), wrap->req(), *path, mode, MkdirCallback);
  if (result < 0) {
    wrap->Reject(uv_strerror(result));
    wrap->Cleanup();
  }
}

static void SimpleCallback(uv_fs_t* req) {
  FsReqWrap* wrap = static_cast<FsReqWrap*>(req->data);

  if (req->result < 0) {
    wrap->Reject(uv_strerror(static_cast<int>(req->result)));
  } else {
    wrap->Resolve(v8::Undefined(wrap->env()->isolate()));
  }

  wrap->Cleanup();
}

// unlink(path) -> Promise<void>
static void Unlink(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);

  if (args.Length() < 1 || !args[0]->IsString()) {
    THROW_ERR_INVALID_ARG_TYPE(isolate, "path must be a string");
    return;
  }

  String::Utf8Value path(isolate, args[0]);

  Local<Promise::Resolver> resolver = Promise::Resolver::New(context).ToLocalChecked();
  args.GetReturnValue().Set(resolver->GetPromise());

  FsReqWrap* wrap = new FsReqWrap(env, resolver);

  int result = uv_fs_unlink(env->event_loop(), wrap->req(), *path, SimpleCallback);
  if (result < 0) {
    wrap->Reject(uv_strerror(result));
    wrap->Cleanup();
  }
}

// rmdir(path) -> Promise<void>
static void Rmdir(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);

  if (args.Length() < 1 || !args[0]->IsString()) {
    THROW_ERR_INVALID_ARG_TYPE(isolate, "path must be a string");
    return;
  }

  String::Utf8Value path(isolate, args[0]);

  Local<Promise::Resolver> resolver = Promise::Resolver::New(context).ToLocalChecked();
  args.GetReturnValue().Set(resolver->GetPromise());

  FsReqWrap* wrap = new FsReqWrap(env, resolver);

  int result = uv_fs_rmdir(env->event_loop(), wrap->req(), *path, SimpleCallback);
  if (result < 0) {
    wrap->Reject(uv_strerror(result));
    wrap->Cleanup();
  }
}

// rename(oldPath, newPath) -> Promise<void>
static void Rename(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);

  if (args.Length() < 2 || !args[0]->IsString() || !args[1]->IsString()) {
    THROW_ERR_INVALID_ARG_TYPE(isolate, "paths must be strings");
    return;
  }

  String::Utf8Value old_path(isolate, args[0]);
  String::Utf8Value new_path(isolate, args[1]);

  Local<Promise::Resolver> resolver = Promise::Resolver::New(context).ToLocalChecked();
  args.GetReturnValue().Set(resolver->GetPromise());

  FsReqWrap* wrap = new FsReqWrap(env, resolver);

  int result = uv_fs_rename(env->event_loop(), wrap->req(), *old_path, *new_path, SimpleCallback);
  if (result < 0) {
    wrap->Reject(uv_strerror(result));
    wrap->Cleanup();
  }
}

static void CreatePerIsolateProperties(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();

  SetMethod(isolate, target, "readFileSync", ReadFileSync);
  SetMethod(isolate, target, "writeFileSync", WriteFileSync);
  SetMethod(isolate, target, "existsSync", ExistsSync);
  SetMethod(isolate, target, "statSync", StatSync);
  SetMethod(isolate, target, "readdirSync", ReaddirSync);
  SetMethod(isolate, target, "mkdirSync", MkdirSync);
  SetMethod(isolate, target, "unlinkSync", UnlinkSync);
  SetMethod(isolate, target, "rmdirSync", RmdirSync);
  SetMethod(isolate, target, "renameSync", RenameSync);
  SetMethod(isolate, target, "realpathSync", RealpathSync);

  SetMethod(isolate, target, "readFile", ReadFile);
  SetMethod(isolate, target, "writeFile", WriteFile);
  SetMethod(isolate, target, "stat", Stat);
  SetMethod(isolate, target, "readdir", Readdir);
  SetMethod(isolate, target, "mkdir", Mkdir);
  SetMethod(isolate, target, "unlink", Unlink);
  SetMethod(isolate, target, "rmdir", Rmdir);
  SetMethod(isolate, target, "rename", Rename);
}

static void CreatePerContextProperties(Local<Object> target, Local<Context> context) {}

NYX_BINDING_PER_ISOLATE_INIT(fs, CreatePerIsolateProperties)
NYX_BINDING_CONTEXT_AWARE(fs, CreatePerContextProperties)

}  // namespace nyx
