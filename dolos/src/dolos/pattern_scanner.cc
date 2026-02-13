#include "dolos/pattern_scanner.h"
#include "dolos/pipe_log.h"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

#include <tlhelp32.h>

// TODO: decouple eidolon decryption from pattern scanner
extern "C" {
uint64_t __fastcall DecryptAddress(uint64_t ptr);
extern uint64_t DecryptGadget;
}

inline void SetDecryptGadget(uintptr_t va) {
  DecryptGadget = va;
}

namespace dolos {

std::optional<ParsedPattern> ParsedPattern::Parse(std::string_view pattern) {
  ParsedPattern result;
  result.has_offset_marker = false;
  result.offset_position = 0;

  std::size_t i = 0;
  while (i < pattern.size()) {
    // Skip whitespace
    while (i < pattern.size() && std::isspace(static_cast<unsigned char>(pattern[i]))) {
      ++i;
    }
    if (i >= pattern.size()) {
      break;
    }

    // offset marker
    if (pattern[i] == '^') {
      if (result.has_offset_marker) {
        return std::nullopt;
      }
      result.offset_position = result.bytes.size();
      result.has_offset_marker = true;
      result.bytes.push_back(0);
      result.mask.push_back(0);
      ++i;
      continue;
    }

    // wildcard
    if (pattern[i] == '?') {
      result.bytes.push_back(0);
      result.mask.push_back(0);
      ++i;
      if (i < pattern.size() && pattern[i] == '?') {
        ++i;
      }
      continue;
    }

    if (i + 1 < pattern.size() && std::isxdigit(static_cast<unsigned char>(pattern[i])) &&
        std::isxdigit(static_cast<unsigned char>(pattern[i + 1]))) {
      std::uint8_t byte = 0;
      auto [ptr, ec] = std::from_chars(pattern.data() + i, pattern.data() + i + 2, byte, 16);
      if (ec != std::errc()) {
        return std::nullopt;
      }
      result.bytes.push_back(byte);
      result.mask.push_back(1);
      i += 2;
      continue;
    }

    return std::nullopt;
  }

  if (result.bytes.empty()) {
    return std::nullopt;
  }

  return result;
}

PatternScanner::PatternScanner() = default;

PatternScanner::~PatternScanner() {
  ResumeAllThreads();
}

bool PatternScanner::Initialize() {
  HMODULE hModule = GetModuleHandle(nullptr);
  if (!hModule) {
    PIPE_LOG_ERROR("[PatternScanner] Failed to get module handle");
    return false;
  }

  if (!SuspendAllThreads()) {
    PIPE_LOG_WARN("[PatternScanner] Failed to suspend threads, scanning anyway");
  }

  module_base_ = reinterpret_cast<std::uintptr_t>(hModule);

  auto* dos_header = reinterpret_cast<IMAGE_DOS_HEADER*>(module_base_);
  if (dos_header->e_magic != IMAGE_DOS_SIGNATURE) {
    PIPE_LOG_ERROR("[PatternScanner] Invalid DOS signature");
    return false;
  }

  auto* nt_headers = reinterpret_cast<IMAGE_NT_HEADERS*>(module_base_ + dos_header->e_lfanew);
  if (nt_headers->Signature != IMAGE_NT_SIGNATURE) {
    PIPE_LOG_ERROR("[PatternScanner] Invalid NT signature");
    return false;
  }

  module_size_ = nt_headers->OptionalHeader.SizeOfImage;
  PIPE_LOG_DEBUG("[PatternScanner] Module base: {:p}, size: {:X}", reinterpret_cast<void*>(module_base_), module_size_);

  bool success = CopyExecutableMemory();

  ResumeAllThreads();

  if (!success) {
    PIPE_LOG_ERROR("[PatternScanner] Failed to copy executable memory");
    return false;
  }

  PIPE_LOG("[PatternScanner] Copied {} bytes of executable memory", buffer_.size());
  return true;
}

bool PatternScanner::SuspendAllThreads() {
  HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
  if (snapshot == INVALID_HANDLE_VALUE) {
    return false;
  }

  DWORD current_thread = GetCurrentThreadId();
  DWORD current_process = GetCurrentProcessId();

  THREADENTRY32 te;
  te.dwSize = sizeof(THREADENTRY32);

  if (Thread32First(snapshot, &te)) {
    do {
      if (te.th32OwnerProcessID == current_process && te.th32ThreadID != current_thread) {
        HANDLE thread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
        if (thread) {
          if (SuspendThread(thread) != static_cast<DWORD>(-1)) {
            suspended_threads_.push_back(te.th32ThreadID);
          }
          CloseHandle(thread);
        }
      }
      te.dwSize = sizeof(THREADENTRY32);
    } while (Thread32Next(snapshot, &te));
  }

  CloseHandle(snapshot);
  PIPE_LOG_DEBUG("[PatternScanner] Suspended {} threads", suspended_threads_.size());
  return true;
}

void PatternScanner::ResumeAllThreads() {
  for (DWORD thread_id : suspended_threads_) {
    HANDLE thread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, thread_id);
    if (thread) {
      ResumeThread(thread);
      CloseHandle(thread);
    }
  }

  if (!suspended_threads_.empty()) {
    PIPE_LOG_DEBUG("[PatternScanner] Resumed {} threads", suspended_threads_.size());
  }
  suspended_threads_.clear();
}

namespace {

// Page protections that allow reading
constexpr DWORD kReadableProtections = PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE |
                                       PAGE_EXECUTE_WRITECOPY | PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY;

bool IsReadableProtection(DWORD protect) {
  return ((protect & kReadableProtections) == protect);
}

std::uintptr_t FindDecryptGadget(std::uintptr_t base, std::size_t size) {
  // decrypt gadget pattern: 48 8B 00 C3 (mov rax, [rax]; ret)
  constexpr std::uint8_t kGadgetPattern[] = {0x48, 0x8B, 0x00, 0xC3};
  constexpr std::size_t kGadgetSize = sizeof(kGadgetPattern);

  std::uintptr_t addr = base;
  std::uintptr_t end_addr = base + size;

  while (addr < end_addr) {
    MEMORY_BASIC_INFORMATION mbi = {};
    if (VirtualQuery(reinterpret_cast<void*>(addr), &mbi, sizeof(mbi)) == 0) {
      break;
    }

    if (mbi.BaseAddress && mbi.RegionSize && mbi.State == MEM_COMMIT && IsReadableProtection(mbi.Protect)) {
      const auto* region_start = static_cast<const std::uint8_t*>(mbi.BaseAddress);
      std::size_t region_size = mbi.RegionSize;

      if (reinterpret_cast<std::uintptr_t>(region_start) < base) {
        std::size_t skip = base - reinterpret_cast<std::uintptr_t>(region_start);
        region_start += skip;
        region_size -= skip;
      }
      if (reinterpret_cast<std::uintptr_t>(region_start) + region_size > end_addr) {
        region_size = end_addr - reinterpret_cast<std::uintptr_t>(region_start);
      }

      for (std::size_t i = 0; i + kGadgetSize <= region_size; ++i) {
        if (std::memcmp(region_start + i, kGadgetPattern, kGadgetSize) == 0) {
          return reinterpret_cast<std::uintptr_t>(region_start) + i;
        }
      }
    }

    std::uintptr_t next = reinterpret_cast<std::uintptr_t>(mbi.BaseAddress) + mbi.RegionSize;
    addr = (next > addr) ? next : end_addr;
  }

  return 0;
}

}  // namespace

bool PatternScanner::CopyExecutableMemory() {
  buffer_.clear();
  buffer_.resize(module_size_, 0);
  sections_.clear();

  std::uintptr_t end_addr = module_base_ + module_size_;

  std::uintptr_t gadget_addr = FindDecryptGadget(module_base_, module_size_);
  if (gadget_addr != 0) {
    PIPE_LOG_DEBUG("[PatternScanner] Found decrypt gadget at {:p}", reinterpret_cast<void*>(gadget_addr));
    SetDecryptGadget(gadget_addr);
    for (std::size_t i = module_base_; i < module_base_ + module_size_; i += 0x100) {
      DecryptAddress(i);
    }
  } else {
    PIPE_LOG_ERROR("[PatternScanner] No decrypt gadget found, aborting");
    return false;
  }

  std::uintptr_t addr = module_base_;
  std::size_t copied_regions = 0;
  std::size_t decrypted_regions = 0;

  while (addr < end_addr) {
    MEMORY_BASIC_INFORMATION mbi = {};
    if (VirtualQuery(reinterpret_cast<void*>(addr), &mbi, sizeof(mbi)) == 0) {
      break;
    }

    if (DecryptGadget && mbi.BaseAddress && mbi.RegionSize && mbi.State == MEM_COMMIT &&
        (mbi.Protect & PAGE_NOACCESS)) {
      DecryptAddress(reinterpret_cast<std::uint64_t>(mbi.BaseAddress));
      VirtualQuery(reinterpret_cast<void*>(addr), &mbi, sizeof(mbi));
      ++decrypted_regions;
    }

    if (mbi.BaseAddress && mbi.RegionSize && mbi.State == MEM_COMMIT && IsReadableProtection(mbi.Protect)) {
      std::uintptr_t region_start = reinterpret_cast<std::uintptr_t>(mbi.BaseAddress);
      std::uintptr_t region_end = region_start + mbi.RegionSize;

      if (region_start < module_base_) {
        region_start = module_base_;
      }
      if (region_end > end_addr) {
        region_end = end_addr;
      }

      std::size_t offset = region_start - module_base_;
      std::size_t size = region_end - region_start;

      if (size > 0 && offset + size <= buffer_.size()) {
        std::memcpy(buffer_.data() + offset, reinterpret_cast<void*>(region_start), size);
        ++copied_regions;

        SectionInfo sec = {};
        sec.virtual_address = static_cast<std::uint32_t>(offset);
        sec.virtual_size = static_cast<std::uint32_t>(size);
        sec.characteristics = PEBuilder::MapProtectionToCharacteristics(mbi.Protect);
        PEBuilder::DeriveSectionName(sec, sections_.size());
        sections_.push_back(sec);
      }
    }

    std::uintptr_t next = reinterpret_cast<std::uintptr_t>(mbi.BaseAddress) + mbi.RegionSize;
    addr = (next > addr) ? next : end_addr;
  }

  PIPE_LOG_DEBUG("[PatternScanner] Copied {} memory regions ({} decrypted), {} sections",
                 copied_regions,
                 decrypted_regions,
                 sections_.size());
  return copied_regions > 0;
}

std::size_t PatternScanner::ScanBuffer(const ParsedPattern& pattern) const {
  if (pattern.bytes.empty() || buffer_.size() < pattern.bytes.size()) {
    return SIZE_MAX;
  }

  const std::size_t scan_end = buffer_.size() - pattern.bytes.size();
  const std::size_t pattern_size = pattern.bytes.size();
  const std::uint8_t* buffer_data = buffer_.data();
  const std::uint8_t* pattern_data = pattern.bytes.data();
  const std::uint8_t* mask_data = pattern.mask.data();

  for (std::size_t i = 0; i <= scan_end; ++i) {
    bool match = true;
    for (std::size_t j = 0; j < pattern_size; ++j) {
      if (mask_data[j] && buffer_data[i + j] != pattern_data[j]) {
        match = false;
        break;
      }
    }
    if (match) {
      return i;
    }
  }

  return SIZE_MAX;
}

std::uintptr_t PatternScanner::Scan(const ParsedPattern& pattern) const {
  std::size_t offset = ScanBuffer(pattern);
  if (offset == SIZE_MAX) {
    return 0;
  }
  return module_base_ + offset;
}

void* PatternScanner::ResolveOffset(std::uintptr_t match_addr,
                                    std::size_t offset_pos,
                                    OffsetType type,
                                    const std::uint8_t* data) {
  const std::uint8_t* offset_ptr = data + offset_pos;

  switch (type) {
    case OffsetType::Absolute: {
      // direct 8-byte pointer value
      return *reinterpret_cast<void* const*>(offset_ptr);
    }

    case OffsetType::Relative32:
    case OffsetType::Relative32Add: {
      // 32-bit RIP-relative offset
      // instruction address + offset position + 4 (size of offset) + relative offset
      std::int32_t rel = *reinterpret_cast<const std::int32_t*>(offset_ptr);
      return reinterpret_cast<void*>(match_addr + offset_pos + 4 + rel);
    }
  }

  return nullptr;
}

bool PatternScanner::DumpBuffer(const std::string& path) const {
  if (buffer_.empty()) {
    PIPE_LOG_ERROR("[PatternScanner] Cannot dump empty buffer");
    return false;
  }

  std::size_t last_slash = path.find_last_of("/\\");
  if (last_slash != std::string::npos) {
    std::string dir = path.substr(0, last_slash);
    std::filesystem::create_directories(dir);
  }

  std::ofstream file(path, std::ios::binary | std::ios::trunc);
  if (!file) {
    PIPE_LOG_ERROR("[PatternScanner] Failed to create dump file: {}", path);
    return false;
  }

  file.write(reinterpret_cast<const char*>(buffer_.data()), buffer_.size());

  if (!file) {
    PIPE_LOG_ERROR("[PatternScanner] Failed to write dump file");
    return false;
  }

  PIPE_LOG_DEBUG("[PatternScanner] Dumped {} bytes to {}", buffer_.size(), path);
  return true;
}

bool PatternScanner::ScanAll(std::vector<SignatureDef>& signatures) {
  std::size_t found_count = 0;

  for (auto& sig : signatures) {
    if (!sig.parsed.has_value()) {
      sig.parsed = ParsedPattern::Parse(sig.pattern);
      if (!sig.parsed.has_value()) {
        PIPE_LOG_ERROR("[PatternScanner] Failed to parse pattern for {}: {}", sig.name, std::string(sig.pattern));
        continue;
      }
    }

    const auto& pattern = sig.parsed.value();

    std::size_t buffer_offset = ScanBuffer(pattern);
    if (buffer_offset == SIZE_MAX) {
      PIPE_LOG_DEBUG("[PatternScanner] Pattern not found: {}", sig.name);
      *sig.target = nullptr;
      continue;
    }

    std::uintptr_t match_addr = module_base_ + buffer_offset;
    void* resolved;

    if (!pattern.has_offset_marker) {
      // No ^ marker: return the match address directly (eg. function prologue)
      resolved = reinterpret_cast<void*>(match_addr);
    } else {
      // Has ^ marker: resolve the relative/absolute offset
      resolved = ResolveOffset(match_addr, pattern.offset_position, sig.offset_type, buffer_.data() + buffer_offset);
    }

    *sig.target = resolved;
    sig.offset = reinterpret_cast<uint64_t>(resolved) - module_base_;
    ++found_count;

    PIPE_LOG_DEBUG("[PatternScanner] {} = {:08X}", sig.name, sig.offset);
  }

  PIPE_LOG_INFO("[PatternScanner] Found {}/{} signatures", found_count, signatures.size());
  return found_count == signatures.size();
}

}  // namespace dolos
