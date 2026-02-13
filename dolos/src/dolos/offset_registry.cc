#include "dolos/offset_registry.h"

namespace dolos {

static std::unordered_map<std::string, void*> g_offsets;

void RegisterOffset(const char* name, void* address) {
  g_offsets[name] = address;
}

const std::unordered_map<std::string, void*>& GetRegisteredOffsets() {
  return g_offsets;
}

void ClearOffsets() {
  g_offsets.clear();
}

}  // namespace dolos
