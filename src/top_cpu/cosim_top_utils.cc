#include "cosim_top_utils.h"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

#include <sys/stat.h>
#include <sys/types.h>

std::string top_env_string(const char* name, const char* default_value)
{
    const char* value = std::getenv(name);
    return value == nullptr || value[0] == '\0' ? std::string(default_value) : std::string(value);
}

uint32_t top_env_u32(const char* name, uint32_t default_value)
{
    const char* value = std::getenv(name);
    if (value == nullptr || value[0] == '\0') {
        return default_value;
    }
    return static_cast<uint32_t>(std::stoul(value, nullptr, 0));
}

uint64_t top_env_u64(const char* name, uint64_t default_value)
{
    const char* value = std::getenv(name);
    if (value == nullptr || value[0] == '\0') {
        return default_value;
    }
    return std::stoull(value, nullptr, 0);
}

bool top_env_enabled(const char* name)
{
    const char* value = std::getenv(name);
    return value != nullptr && value[0] != '\0' && std::strcmp(value, "0") != 0;
}

void top_ensure_directory(const std::string& dir)
{
    if (dir.empty()) {
        return;
    }

    std::string path;
    std::size_t pos = 0;
    if (dir[0] == '/') {
        path = "/";
        pos = 1;
    }

    while (pos <= dir.size()) {
        const std::size_t slash = dir.find('/', pos);
        const std::string part = dir.substr(pos, slash - pos);
        if (!part.empty()) {
            if (!path.empty() && path.back() != '/') {
                path += '/';
            }
            path += part;

            if (mkdir(path.c_str(), 0777) != 0 && errno != EEXIST) {
                throw std::runtime_error("failed to create " + path + ": " + std::strerror(errno));
            }
        }

        if (slash == std::string::npos) {
            break;
        }
        pos = slash + 1;
    }
}

void top_ensure_parent_directory(const std::string& path)
{
    const std::size_t slash = path.find_last_of('/');
    if (slash == std::string::npos) {
        return;
    }
    top_ensure_directory(path.substr(0, slash));
}
