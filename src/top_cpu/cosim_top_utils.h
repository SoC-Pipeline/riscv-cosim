#ifndef TOP_COSIM_TOP_UTILS_H
#define TOP_COSIM_TOP_UTILS_H

#include <cstdint>
#include <string>

std::string top_env_string(const char* name, const char* default_value);
uint32_t top_env_u32(const char* name, uint32_t default_value);
uint64_t top_env_u64(const char* name, uint64_t default_value);
bool top_env_enabled(const char* name);
void top_ensure_directory(const std::string& dir);
void top_ensure_parent_directory(const std::string& path);

#endif
