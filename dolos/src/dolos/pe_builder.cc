#include "dolos/pe_builder.h"
#include "dolos/pipe_log.h"

#include <cstring>
#include <filesystem>
#include <fstream>

namespace dolos {

namespace {

constexpr std::uint32_t AlignUp(std::uint32_t value, std::uint32_t alignment) {
  return (value + alignment - 1) & ~(alignment - 1);
}

}  // namespace

PEBuilder::PEBuilder(std::uintptr_t image_base, std::size_t image_size)
    : image_base_(image_base), image_size_(image_size) {}

void PEBuilder::AddSection(const SectionInfo& section) {
  sections_.push_back(section);
}

std::uint32_t PEBuilder::MapProtectionToCharacteristics(DWORD protect) {
  std::uint32_t chars = 0;

  if (protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)) {
    chars |= IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ;
    if (protect & (PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)) {
      chars |= IMAGE_SCN_MEM_WRITE;
    }
  } else if (protect & (PAGE_READWRITE | PAGE_WRITECOPY)) {
    chars |= IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE;
  } else if (protect & (PAGE_READONLY)) {
    chars |= IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ;
  } else {
    chars |= IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ;
  }

  return chars;
}

void PEBuilder::DeriveSectionName(SectionInfo& sec, std::size_t index) {
  std::memset(sec.name, 0, 8);

  // Name based on characteristics
  if (sec.characteristics & IMAGE_SCN_MEM_EXECUTE) {
    if (index == 0) {
      std::memcpy(sec.name, ".text", 5);
    } else {
      char buf[8];
      std::snprintf(buf, sizeof(buf), ".code%zu", index);
      std::memcpy(sec.name, buf, 7);
    }
  } else if (sec.characteristics & IMAGE_SCN_MEM_WRITE) {
    std::memcpy(sec.name, ".data", 5);
  } else {
    // Read-only data - differentiate by RVA
    if (sec.virtual_address < 0x200000) {
      std::memcpy(sec.name, ".rdata", 6);
    } else {
      std::memcpy(sec.name, ".rodata", 7);
    }
  }
}

void PEBuilder::BuildDosHeader(std::vector<std::uint8_t>& out) {
  IMAGE_DOS_HEADER dos = {};
  dos.e_magic = IMAGE_DOS_SIGNATURE;
  dos.e_cblp = 0x90;
  dos.e_cp = 0x03;
  dos.e_cparhdr = 0x04;
  dos.e_maxalloc = 0xFFFF;
  dos.e_sp = 0xB8;
  dos.e_lfarlc = 0x40;
  dos.e_lfanew = 0x80;

  const auto* ptr = reinterpret_cast<const std::uint8_t*>(&dos);
  out.insert(out.end(), ptr, ptr + sizeof(dos));

  static const std::uint8_t kDosStub[] = {0x0E, 0x1F, 0xBA, 0x0E, 0x00, 0xB4, 0x09, 0xCD, 0x21, 0xB8, 0x01, 0x4C, 0xCD,
                                          0x21, 0x54, 0x68, 0x69, 0x73, 0x20, 0x70, 0x72, 0x6F, 0x67, 0x72, 0x61, 0x6D,
                                          0x20, 0x63, 0x61, 0x6E, 0x6E, 0x6F, 0x74, 0x20, 0x62, 0x65, 0x20, 0x72, 0x75,
                                          0x6E, 0x20, 0x69, 0x6E, 0x20, 0x44, 0x4F, 0x53, 0x20, 0x6D, 0x6F, 0x64, 0x65,
                                          0x2E, 0x0D, 0x0D, 0x0A, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  out.insert(out.end(), kDosStub, kDosStub + sizeof(kDosStub));

  while (out.size() < 0x80) {
    out.push_back(0);
  }
}

void PEBuilder::BuildNtHeaders(std::vector<std::uint8_t>& out) {
  IMAGE_NT_HEADERS64 nt = {};

  nt.Signature = IMAGE_NT_SIGNATURE;

  nt.FileHeader.Machine = IMAGE_FILE_MACHINE_AMD64;
  nt.FileHeader.NumberOfSections = static_cast<WORD>(sections_.size());
  nt.FileHeader.TimeDateStamp = 0;
  nt.FileHeader.PointerToSymbolTable = 0;
  nt.FileHeader.NumberOfSymbols = 0;
  nt.FileHeader.SizeOfOptionalHeader = sizeof(IMAGE_OPTIONAL_HEADER64);
  nt.FileHeader.Characteristics = IMAGE_FILE_EXECUTABLE_IMAGE | IMAGE_FILE_LARGE_ADDRESS_AWARE | IMAGE_FILE_DLL;

  nt.OptionalHeader.Magic = IMAGE_NT_OPTIONAL_HDR64_MAGIC;
  nt.OptionalHeader.MajorLinkerVersion = 14;
  nt.OptionalHeader.MinorLinkerVersion = 0;

  std::uint32_t size_of_code = 0;
  std::uint32_t size_of_initialized_data = 0;
  std::uint32_t base_of_code = 0;

  for (const auto& sec : sections_) {
    if (sec.characteristics & IMAGE_SCN_MEM_EXECUTE) {
      size_of_code += sec.virtual_size;
      if (base_of_code == 0) {
        base_of_code = sec.virtual_address;
      }
    } else {
      size_of_initialized_data += sec.virtual_size;
    }
  }

  nt.OptionalHeader.SizeOfCode = size_of_code;
  nt.OptionalHeader.SizeOfInitializedData = size_of_initialized_data;
  nt.OptionalHeader.SizeOfUninitializedData = 0;
  nt.OptionalHeader.AddressOfEntryPoint = base_of_code;
  nt.OptionalHeader.BaseOfCode = base_of_code;

  nt.OptionalHeader.ImageBase = image_base_;
  nt.OptionalHeader.SectionAlignment = 0x1000;
  nt.OptionalHeader.FileAlignment = 0x1000;
  nt.OptionalHeader.MajorOperatingSystemVersion = 6;
  nt.OptionalHeader.MinorOperatingSystemVersion = 0;
  nt.OptionalHeader.MajorImageVersion = 0;
  nt.OptionalHeader.MinorImageVersion = 0;
  nt.OptionalHeader.MajorSubsystemVersion = 6;
  nt.OptionalHeader.MinorSubsystemVersion = 0;
  nt.OptionalHeader.Win32VersionValue = 0;
  nt.OptionalHeader.SizeOfImage = static_cast<DWORD>(image_size_);

  std::size_t headers_size = 0x80 + sizeof(IMAGE_NT_HEADERS64) + sections_.size() * sizeof(IMAGE_SECTION_HEADER);
  nt.OptionalHeader.SizeOfHeaders = AlignUp(static_cast<std::uint32_t>(headers_size), 0x1000);

  nt.OptionalHeader.CheckSum = 0;
  nt.OptionalHeader.Subsystem = IMAGE_SUBSYSTEM_WINDOWS_GUI;
  nt.OptionalHeader.DllCharacteristics = IMAGE_DLLCHARACTERISTICS_HIGH_ENTROPY_VA |
                                         IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE | IMAGE_DLLCHARACTERISTICS_NX_COMPAT;
  nt.OptionalHeader.SizeOfStackReserve = 0x100000;
  nt.OptionalHeader.SizeOfStackCommit = 0x1000;
  nt.OptionalHeader.SizeOfHeapReserve = 0x100000;
  nt.OptionalHeader.SizeOfHeapCommit = 0x1000;
  nt.OptionalHeader.LoaderFlags = 0;
  nt.OptionalHeader.NumberOfRvaAndSizes = IMAGE_NUMBEROF_DIRECTORY_ENTRIES;

  const auto* ptr = reinterpret_cast<const std::uint8_t*>(&nt);
  out.insert(out.end(), ptr, ptr + sizeof(nt));
}

void PEBuilder::BuildSectionHeaders(std::vector<std::uint8_t>& out) {
  for (const auto& sec : sections_) {
    IMAGE_SECTION_HEADER hdr = {};
    std::memcpy(hdr.Name, sec.name, 8);
    hdr.Misc.VirtualSize = sec.virtual_size;
    hdr.VirtualAddress = sec.virtual_address;
    hdr.SizeOfRawData = AlignUp(sec.virtual_size, 0x1000);
    hdr.PointerToRawData = sec.virtual_address;
    hdr.PointerToRelocations = 0;
    hdr.PointerToLinenumbers = 0;
    hdr.NumberOfRelocations = 0;
    hdr.NumberOfLinenumbers = 0;
    hdr.Characteristics = sec.characteristics;

    const auto* ptr = reinterpret_cast<const std::uint8_t*>(&hdr);
    out.insert(out.end(), ptr, ptr + sizeof(hdr));
  }
}

bool PEBuilder::Build(const std::vector<std::uint8_t>& raw_buffer, std::vector<std::uint8_t>& output) {
  if (sections_.empty()) {
    PIPE_LOG_ERROR("[PEBuilder] No sections defined");
    return false;
  }

  output.clear();

  BuildDosHeader(output);
  BuildNtHeaders(output);
  BuildSectionHeaders(output);

  std::uint32_t first_section_rva = sections_[0].virtual_address;
  while (output.size() < first_section_rva) {
    output.push_back(0);
  }

  if (first_section_rva < raw_buffer.size()) {
    output.insert(output.end(), raw_buffer.begin() + first_section_rva, raw_buffer.end());
  }

  PIPE_LOG_DEBUG("[PEBuilder] Built PE with {} sections, {} bytes total", sections_.size(), output.size());
  return true;
}

bool PEBuilder::WriteExecutable(const std::vector<std::uint8_t>& raw_buffer, const std::string& path) {
  std::vector<std::uint8_t> pe_data;
  if (!Build(raw_buffer, pe_data)) {
    return false;
  }

  std::size_t last_slash = path.find_last_of("/\\");
  if (last_slash != std::string::npos) {
    std::string dir = path.substr(0, last_slash);
    std::filesystem::create_directories(dir);
  }

  std::ofstream file(path, std::ios::binary | std::ios::trunc);
  if (!file) {
    PIPE_LOG_ERROR("[PEBuilder] Failed to create output file: {}", path);
    return false;
  }

  file.write(reinterpret_cast<const char*>(pe_data.data()), pe_data.size());

  if (!file) {
    PIPE_LOG_ERROR("[PEBuilder] Failed to write output file");
    return false;
  }

  PIPE_LOG_DEBUG("[PEBuilder] Wrote PE dump to {}", path);
  return true;
}

}  // namespace dolos
