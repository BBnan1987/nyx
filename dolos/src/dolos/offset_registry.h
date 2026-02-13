#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

namespace dolos {

void RegisterOffset(const char* name, void* address);
const std::unordered_map<std::string, void*>& GetRegisteredOffsets();
void ClearOffsets();

}  // namespace dolos
