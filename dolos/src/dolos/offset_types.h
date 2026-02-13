#pragma once

#include <cstddef>
#include <cstdint>

namespace dolos {

enum class OffsetType {
  Absolute,      // Direct pointer value (8 bytes)
  Relative32,    // RIP-relative 32-bit offset (call/jmp instructions)
  Relative32Add  // RIP-relative data reference (mov/lea to memory)
};

}  // namespace dolos
