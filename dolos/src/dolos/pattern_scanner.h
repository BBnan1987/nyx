#pragma once

#include "dolos/offset_types.h"
#include "dolos/pe_builder.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <windows.h>

namespace dolos {

struct ParsedPattern {
  std::vector<std::uint8_t> bytes;  // Pattern bytes (0 for wildcards)
  std::vector<std::uint8_t> mask;   // 1 = must match, 0 = wildcard
  std::size_t offset_position;      // Position of ^ marker in pattern
  bool has_offset_marker;           // Whether ^ was found

  // Parse a pattern string like "8B 1D ^ ? ? ? FF C8"
  static std::optional<ParsedPattern> Parse(std::string_view pattern);
};

struct SignatureDef {
  const char* name;          // Offset name for logging/caching
  std::string_view pattern;  // Pattern string to search for
  OffsetType offset_type;    // How to resolve the offset
  void** target;             // Pointer to store the result
  uint64_t offset;           // Relative offset

  std::optional<ParsedPattern> parsed;
};

class PatternScanner {
 public:
  PatternScanner();
  ~PatternScanner();

  PatternScanner(const PatternScanner&) = delete;
  PatternScanner& operator=(const PatternScanner&) = delete;

  bool Initialize();

  std::uintptr_t Scan(const ParsedPattern& pattern) const;
  bool ScanAll(std::vector<SignatureDef>& signatures);

  // Resolve an offset value based on the offset type
  // match_addr: address where pattern was found (in process memory)
  // offset_pos: position of ^ marker within the pattern
  // type: how to interpret the offset bytes
  // data: pointer to the matched bytes (in local buffer)
  static void* ResolveOffset(std::uintptr_t match_addr,
                             std::size_t offset_pos,
                             OffsetType type,
                             const std::uint8_t* data);

  std::uintptr_t module_base() const { return module_base_; }
  std::size_t module_size() const { return module_size_; }
  const std::vector<std::uint8_t>& buffer() const { return buffer_; }
  const std::vector<SectionInfo>& sections() const { return sections_; }

  bool DumpBuffer(const std::string& path) const;

 private:
  bool SuspendAllThreads();
  void ResumeAllThreads();

  bool CopyExecutableMemory();

  std::size_t ScanBuffer(const ParsedPattern& pattern) const;

  std::uintptr_t module_base_ = 0;
  std::size_t module_size_ = 0;
  std::vector<std::uint8_t> buffer_;
  std::vector<DWORD> suspended_threads_;
  std::vector<SectionInfo> sections_;
};

}  // namespace dolos
