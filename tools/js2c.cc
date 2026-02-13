#include <uv.h>
#include <algorithm>
#include <array>
#include <functional>
#include <iostream>
#include <map>
#include <vector>

static bool is_verbose = false;

typedef std::vector<std::string> FileList;
typedef std::map<std::string, FileList> FileMap;

static void PrintUvError(const char* syscall, const char* filename, int error) {
  fprintf(stderr, "[%s] %s: %s\n", syscall, filename, uv_strerror(error));
}

static int GetStats(const char* path, std::function<void(const uv_stat_t*)> func) {
  uv_fs_t req;
  int r = uv_fs_stat(nullptr, &req, path, nullptr);
  if (r == 0) {
    func(static_cast<const uv_stat_t*>(req.ptr));
  }
  uv_fs_req_cleanup(&req);
  return r;
}

static bool IsDirectory(const std::string& filename, int* error) {
  bool result = false;
  *error = GetStats(filename.c_str(), [&](const uv_stat_t* stats) { result = !!(stats->st_mode & S_IFDIR); });
  if (*error != 0) {
    PrintUvError("stat", filename.c_str(), *error);
  }
  return result;
}

static size_t GetFileSize(const std::string& filename, int* error) {
  size_t result = 0;
  *error = GetStats(filename.c_str(), [&](const uv_stat_t* stats) { result = stats->st_size; });
  return result;
}

static bool SearchFiles(const std::string& dir, FileMap* file_map, std::string_view extension) {
  uv_fs_t scan_req;
  int result = uv_fs_scandir(nullptr, &scan_req, dir.c_str(), 0, nullptr);
  bool errored = false;
  if (result < 0) {
    PrintUvError("scandir", dir.c_str(), result);
    errored = true;
  } else {
    auto it = file_map->insert({std::string(extension), FileList()}).first;
    FileList& files = it->second;
    files.reserve(files.size() + result);
    uv_dirent_t dent;
    while (true) {
      result = uv_fs_scandir_next(&scan_req, &dent);
      if (result == UV_EOF) {
        break;
      }

      if (result != 0) {
        PrintUvError("scandir_next", dir.c_str(), result);
        errored = true;
        break;
      }

      std::string path = dir + '/' + dent.name;
      if (path.ends_with(extension)) {
        files.emplace_back(path);
        continue;
      }
      if (!IsDirectory(path, &result)) {
        if (result == 0) {
          continue;
        } else {
          errored = true;
          break;
        }
      }

      if (!SearchFiles(path, file_map, extension)) {
        errored = true;
        break;
      }
    }
  }

  uv_fs_req_cleanup(&scan_req);
  return !errored;
}

constexpr std::string_view kMjsSuffix = ".mjs";
constexpr std::string_view kJsSuffix = ".js";
constexpr std::string_view kLibPrefix = "lib/";

constexpr std::string_view HasAllowedExtensions(const std::string_view filename) {
  for (const auto& ext : {kJsSuffix, kMjsSuffix}) {
    if (filename.ends_with(ext)) {
      return ext;
    }
  }
  return {};
}

using Fragment = std::vector<char>;
using Fragments = std::vector<Fragment>;

static Fragment Join(const Fragments& fragments, const std::string& separator) {
  size_t length = separator.size() * (fragments.size() - 1);
  for (size_t i = 0; i < fragments.size(); ++i) {
    length += fragments[i].size();
  }
  Fragment buf(length, 0);
  size_t cursor = 0;
  for (size_t i = 0; i < fragments.size(); ++i) {
    const Fragment& fragment = fragments[i];
    if (i != 0) {
      memcpy(buf.data() + cursor, separator.c_str(), separator.size());
      cursor += separator.size();
    }
    memcpy(buf.data() + cursor, fragment.data(), fragment.size());
    cursor += fragment.size();
  }
  buf.resize(cursor);
  return buf;
}

const char* kTemplate = R"(
#include "nyx/env.h"
#include "nyx/builtins.h"
// #include "external_reference.h"

namespace nyx {

%.*s
const BuiltinSourceMap global_source_map {
  BuiltinSourceMap {
%.*s
  }
};

void BuiltinLoader::LoadJavaScriptSource() {
  source_ = global_source_map;
}

// void RegisterExternalReferencesForInternalizedBuiltinCode(ExternalReferenceRegistry* registry) {
/*
%.*s
*/
// }

}  // namespace nyx
)";

// Template for external builtins (used with --external flag)
const char* kExternalTemplate = R"(
#include "%s"
#include <nyx/builtins.h>
#include <nyx/extension.h>

namespace %s {

%.*s
static const nyx::BuiltinSourceMap source_map {
%.*s
};

const nyx::BuiltinSourceMap& GetBuiltinSourceMap() {
  return source_map;
}

void RegisterBuiltins() {
  nyx::RegisterBuiltinSourceMap(&source_map);
}

}  // namespace %s
)";

const char* kExternalHeaderTemplate = R"(#pragma once

#include <map>
#include <string>

namespace nyx {
class UnionBytes;
}

namespace %s {

// Get the builtin source map for this library
// Can be used for direct lookups without registration
const std::map<std::string, nyx::UnionBytes>& GetBuiltinSourceMap();

// Register all JavaScript builtins from this library
// Call this before nyx::Start()
void RegisterBuiltins();

}  // namespace %s
)";

Fragment Format(const Fragments& definitions, const Fragments& initializers, const Fragments& registrations) {
  Fragment def_buf = Join(definitions, "\n");
  size_t def_size = def_buf.size();
  Fragment init_buf = Join(initializers, "\n");
  size_t init_size = init_buf.size();
  Fragment reg_buf = Join(registrations, "\n");
  size_t reg_size = reg_buf.size();

  size_t result_size = def_size + init_size + reg_size + strlen(kTemplate) + 100;
  Fragment result(result_size, 0);
  int r = snprintf(result.data(),
                   result_size,
                   kTemplate,
                   static_cast<int>(def_size),
                   def_buf.data(),
                   static_cast<int>(init_size),
                   init_buf.data(),
                   static_cast<int>(reg_size),
                   reg_buf.data());
  result.resize(r);
  return result;
}

Fragment FormatExternal(const Fragments& definitions,
                        const Fragments& initializers,
                        const std::string& ns,
                        const std::string& header_name) {
  Fragment def_buf = Join(definitions, "\n");
  size_t def_size = def_buf.size();
  Fragment init_buf = Join(initializers, "\n");
  size_t init_size = init_buf.size();

  size_t result_size = def_size + init_size + strlen(kExternalTemplate) + ns.size() * 3 + header_name.size() + 200;
  Fragment result(result_size, 0);
  int r = snprintf(result.data(),
                   result_size,
                   kExternalTemplate,
                   header_name.c_str(),
                   ns.c_str(),
                   static_cast<int>(def_size),
                   def_buf.data(),
                   static_cast<int>(init_size),
                   init_buf.data(),
                   ns.c_str());
  result.resize(r);
  return result;
}

Fragment FormatExternalHeader(const std::string& ns) {
  size_t result_size = strlen(kExternalHeaderTemplate) + ns.size() * 2 + 100;
  Fragment result(result_size, 0);
  int r = snprintf(result.data(), result_size, kExternalHeaderTemplate, ns.c_str(), ns.c_str());
  result.resize(r);
  return result;
}

Fragment ReadFileSync(const char* path, size_t size, int* error) {
  uv_fs_t req;

  uv_file file = uv_fs_open(nullptr, &req, path, O_RDONLY, 0, nullptr);
  if (req.result < 0) {
    uv_fs_req_cleanup(&req);
    *error = req.result;
    return Fragment();
  }
  uv_fs_req_cleanup(&req);

  Fragment contents(size);
  size_t offset = 0;

  while (offset < size) {
    uv_buf_t buf = uv_buf_init(contents.data() + offset, size - offset);
    int bytes_read = uv_fs_read(nullptr, &req, file, &buf, 1, offset, nullptr);
    offset += bytes_read;
    *error = req.result;
    uv_fs_req_cleanup(&req);
    if (*error < 0) {
      uv_fs_close(nullptr, &req, file, nullptr);
      return Fragment();
    }
    if (bytes_read <= 0) {
      break;
    }
  }

  *error = uv_fs_close(nullptr, &req, file, nullptr);
  return contents;
}

int WriteFileSync(const Fragment& out, const char* path) {
  uv_fs_t req;
  uv_file file =
      uv_fs_open(nullptr, &req, path, UV_FS_O_CREAT | UV_FS_O_WRONLY | UV_FS_O_TRUNC, _S_IWRITE | _S_IREAD, nullptr);
  int err = req.result;
  uv_fs_req_cleanup(&req);
  if (err < 0) {
    return err;
  }

  uv_buf_t buf = uv_buf_init(const_cast<char*>(out.data()), out.size());
  err = uv_fs_write(nullptr, &req, file, &buf, 1, 0, nullptr);
  uv_fs_req_cleanup(&req);

  int r = uv_fs_close(nullptr, &req, file, nullptr);
  uv_fs_req_cleanup(&req);
  if (err < 0) {
    return err;
  }
  return r;
}

int WriteIfChanged(const Fragment& out, const std::string& dest) {
  int error = 0;
  size_t size = GetFileSize(dest, &error);
  if (error != 0 && error != UV_ENOENT) {
    return error;
  }

  bool changed = true;
  if (error != UV_ENOENT && size == out.size()) {
    Fragment content = ReadFileSync(dest.c_str(), size, &error);
    if (error == 0) {
      changed = (memcmp(content.data(), out.data(), size) != 0);
    }
  }
  if (!changed) {
    return 0;
  }
  return WriteFileSync(out, dest.c_str());
}

std::string GetFileId(const std::string& filename) {
  size_t end = filename.size();
  size_t start = 0;
  std::string prefix;
  if (filename.ends_with(kMjsSuffix)) {
    end -= kMjsSuffix.size();
  } else if (filename.ends_with(kJsSuffix)) {
    end -= kJsSuffix.size();
  }

  if (filename.starts_with(kLibPrefix)) {
    start = kLibPrefix.size();
    prefix = "";
  }

  return prefix + std::string(filename.begin() + start, filename.begin() + end);
}

std::string GetVariableName(const std::string& id) {
  std::string result = id;
  size_t length = result.size();

  for (size_t i = 0; i < length; ++i) {
    if (result[i] == '.' || result[i] == '-' || result[i] == '/') {
      result[i] = '_';
    }
  }
  return result;
}

std::pair<std::array<char, 382106>, std::array<uint32_t, 65537>> precompute_string() {
  // the string "0,1,2,3,...,65535,".
  std::array<char, 382106> str;
  // the offsets in the string pointing at the beginning of each substring
  std::array<uint32_t, 65537> off;
  off[0] = 0;
  char* p = &str[0];
  constexpr auto const_int_to_str = [](uint16_t value, char* s) -> uint32_t {
    uint32_t index = 0;
    do {
      s[index++] = '0' + (value % 10);
      value /= 10;
    } while (value != 0);

    for (uint32_t i = 0; i < index / 2; ++i) {
      char temp = s[i];
      s[i] = s[index - i - 1];
      s[index - i - 1] = temp;
    }
    s[index] = ',';
    return index + 1;
  };
  for (int i = 0; i < 65536; ++i) {
    size_t offset = const_int_to_str(i, p);
    p += offset;
    off[i + 1] = off[i] + offset;
  }
  return {str, off};
}

const std::string_view GetCode(uint16_t index) {
  static auto [backing_string, offsets] = precompute_string();
  return std::string_view(&backing_string[offsets[index]], offsets[index + 1] - offsets[index]);
}

const char* array_literal_def_template = "static const %s %s_raw[] = ";
constexpr std::string_view array_literal_start = "{\n";
constexpr std::string_view array_literal_end = "\n};\n\n";

template <typename T>
Fragment GetDefinitionImpl(const Fragment& code, const std::string& var) {
  size_t count = code.size();
  constexpr const char* arr_type = "uint16_t";
  constexpr const char* resource_type = "nyx::StaticExternalTwoByteResource";
  const char* literal_def_template = array_literal_def_template;
  constexpr size_t unit = 4;
  size_t def_size = 512 + count * unit;

  Fragment result(def_size, 0);

  int cur = snprintf(result.data(), def_size, literal_def_template, arr_type, var.c_str());

  memcpy(result.data() + cur, array_literal_start.data(), array_literal_start.size());
  cur += array_literal_start.size();

  const uint8_t* ptr = reinterpret_cast<const uint8_t*>(code.data());
  for (size_t i = 0; i < count; ++i) {
    uint16_t ch = static_cast<uint16_t>(ptr[i]);
    if (ch > 127) {
      fprintf(stderr, "In %s, found non-ASCII Latin-1 character at %zu: %d\n", var.c_str(), i, ch);
    }
    std::string_view str = GetCode(ch);
    memcpy(result.data() + cur, str.data(), str.size());
    cur += str.size();
  }

  memcpy(result.data() + cur, array_literal_end.data(), array_literal_end.size());
  cur += array_literal_end.size();

  int end_size = snprintf(result.data() + cur,
                          result.size() - cur,
                          "static %s %s_resource(%s_raw, %zu, nullptr);\n",
                          resource_type,
                          var.c_str(),
                          var.c_str(),
                          count);
  cur += end_size;
  result.resize(cur);
  return result;
}

Fragment GetDefinition(const std::string& var, const Fragment& code) {
  return GetDefinitionImpl<char>(code, var);
}

int AddModule(const std::string& filename, Fragments* definitions, Fragments* initializers, Fragments* registrations) {
  int error = 0;
  size_t file_size = GetFileSize(filename, &error);
  if (error != 0) {
    return error;
  }
  Fragment code = ReadFileSync(filename.c_str(), file_size, &error);
  if (error != 0) {
    return error;
  }
  std::string file_id = GetFileId(filename);
  std::string var = GetVariableName(file_id);

  definitions->emplace_back(GetDefinition(var, code));

  Fragment& init_buf = initializers->emplace_back(Fragment(256, 0));
  int init_size = snprintf(
      init_buf.data(), init_buf.size(), "    {\"%s\", UnionBytes(&%s_resource) },", file_id.c_str(), var.c_str());
  init_buf.resize(init_size);

  Fragment& reg_buf = registrations->emplace_back(Fragment(256, 0));
  int reg_size = snprintf(reg_buf.data(), reg_buf.size(), "  registry->Register(&%s_resource);", var.c_str());
  reg_buf.resize(reg_size);
  return 0;
}

int AddModuleExternal(const std::string& filename, Fragments* definitions, Fragments* initializers) {
  int error = 0;
  size_t file_size = GetFileSize(filename, &error);
  if (error != 0) {
    return error;
  }
  Fragment code = ReadFileSync(filename.c_str(), file_size, &error);
  if (error != 0) {
    return error;
  }
  std::string file_id = GetFileId(filename);
  std::string var = GetVariableName(file_id);

  definitions->emplace_back(GetDefinition(var, code));

  // For external mode, generate BuiltinSourceMap initializers (same as internal)
  Fragment& init_buf = initializers->emplace_back(Fragment(256, 0));
  int init_size = snprintf(
      init_buf.data(), init_buf.size(), "  {\"%s\", nyx::UnionBytes(&%s_resource)},", file_id.c_str(), var.c_str());
  init_buf.resize(init_size);
  return 0;
}

int JS2C(const FileList& js_files, const FileList& mjs_files, const std::string& dest) {
  Fragments definitions;
  definitions.reserve(js_files.size() + mjs_files.size() + 1);
  Fragments initializers;
  initializers.reserve(js_files.size() + mjs_files.size());
  Fragments registrations;
  registrations.reserve(js_files.size() + mjs_files.size() + 1);

  for (const auto& filename : js_files) {
    int r = AddModule(filename, &definitions, &initializers, &registrations);
    if (r != 0) {
      return r;
    }
  }

  for (const auto& filename : mjs_files) {
    int r = AddModule(filename, &definitions, &initializers, &registrations);
    if (r != 0) {
      return r;
    }
  }
  Fragment out = Format(definitions, initializers, registrations);
  return WriteIfChanged(out, dest);
}

int JS2CExternal(const FileList& js_files, const FileList& mjs_files, const std::string& dest, const std::string& ns) {
  Fragments definitions;
  definitions.reserve(js_files.size() + mjs_files.size() + 1);
  Fragments initializers;
  initializers.reserve(js_files.size() + mjs_files.size() + 1);

  for (const auto& filename : js_files) {
    int r = AddModuleExternal(filename, &definitions, &initializers);
    if (r != 0) {
      return r;
    }
  }

  for (const auto& filename : mjs_files) {
    int r = AddModuleExternal(filename, &definitions, &initializers);
    if (r != 0) {
      return r;
    }
  }

  // Generate header file
  std::string header_dest = dest;
  size_t dot_pos = header_dest.rfind('.');
  if (dot_pos != std::string::npos) {
    header_dest = header_dest.substr(0, dot_pos) + ".h";
  } else {
    header_dest += ".h";
  }

  // Extract just the filename for the include
  std::string header_name = header_dest;
  size_t slash_pos = header_name.rfind('/');
  if (slash_pos != std::string::npos) {
    header_name = header_name.substr(slash_pos + 1);
  }
  slash_pos = header_name.rfind('\\');
  if (slash_pos != std::string::npos) {
    header_name = header_name.substr(slash_pos + 1);
  }

  Fragment header_out = FormatExternalHeader(ns);
  int r = WriteIfChanged(header_out, header_dest);
  if (r != 0) {
    return r;
  }

  Fragment out = FormatExternal(definitions, initializers, ns, header_name);
  return WriteIfChanged(out, dest);
}

static int PrintUsage(char* argv0) {
  fprintf(stderr,
          "Usage: %s [--root /path/to/project/root] "
          "[--external --namespace <ns>] "
          "path/to/output.cc path/to/directory "
          "[extra-files ...]\n\n"
          "Options:\n"
          "  --root <path>      Set the working directory\n"
          "  --external         Generate external builtin registration code\n"
          "  --namespace <ns>   C++ namespace for external builtins\n"
          "  --verbose          Print verbose output\n",
          argv0);
  return 1;
}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    return PrintUsage(argv[0]);
  }

  std::vector<std::string> args;
  args.reserve(argc);
  std::string root_dir;
  std::string ns;
  bool external_mode = false;

  for (int i = 1; i < argc; ++i) {
    std::string arg(argv[i]);
    if (arg == "--verbose") {
      is_verbose = true;
    } else if (arg == "--root") {
      if (i == argc - 1) {
        fprintf(stderr, "--root must be followed by a path\n");
        return 1;
      }
      root_dir = argv[++i];
    } else if (arg == "--external") {
      external_mode = true;
    } else if (arg == "--namespace") {
      if (i == argc - 1) {
        fprintf(stderr, "--namespace must be followed by a namespace name\n");
        return 1;
      }
      ns = argv[++i];
    } else {
      args.emplace_back(argv[i]);
    }
  }

  if (external_mode && ns.empty()) {
    fprintf(stderr, "--external requires --namespace\n");
    return 1;
  }

  if (args.size() < 2) {
    return PrintUsage(argv[0]);
  }

  if (!root_dir.empty()) {
    int r = uv_chdir(root_dir.c_str());
    if (r != 0) {
      fprintf(stderr, "Cannot switch to the directory specified by --root\n");
      PrintUvError("chdir", root_dir.c_str(), r);
      return 1;
    }
  }
  std::string output = args[0];

  FileMap file_map;
  for (size_t i = 1; i < args.size(); ++i) {
    int error = 0;
    const std::string& file = args[i];
    if (IsDirectory(file, &error)) {
      if (!SearchFiles(file, &file_map, kJsSuffix) || !SearchFiles(file, &file_map, kMjsSuffix)) {
        return 1;
      }
    } else if (error != 0) {
      return 1;
    } else {
      std::string_view extension = HasAllowedExtensions(file);
      if (extension.size() != 0) {
        auto it = file_map.insert({std::string(extension), FileList()}).first;
        it->second.push_back(file);
      } else {
        fprintf(stderr, "Unsupported file: %s\n", file.c_str());
        return 1;
      }
    }
  }

  auto js_it = file_map.find(".js");
  auto mjs_it = file_map.find(".mjs");

  if (js_it != file_map.end()) {
    std::sort(js_it->second.begin(), js_it->second.end());
  }
  if (mjs_it != file_map.end()) {
    std::sort(mjs_it->second.begin(), mjs_it->second.end());
  }

  FileList empty_list;
  const FileList& js_files = (js_it != file_map.end()) ? js_it->second : empty_list;
  const FileList& mjs_files = (mjs_it != file_map.end()) ? mjs_it->second : empty_list;

  if (external_mode) {
    return JS2CExternal(js_files, mjs_files, output, ns);
  } else {
    return JS2C(js_files, mjs_files, output);
  }
}
