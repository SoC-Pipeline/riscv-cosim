#include "elf_utils.h"

#include <cstdio>
#include <stdexcept>

namespace {

uint64_t read_le_value(const uint8_t* bytes, size_t len)
{
    uint64_t value = 0;
    for (size_t i = 0; i < len; ++i) {
        value |= static_cast<uint64_t>(bytes[i]) << (8 * i);
    }
    return value;
}

}  // namespace

uint64_t read_elf_entry(const std::string& elf_path)
{
    FILE* elf = std::fopen(elf_path.c_str(), "rb");
    if (!elf) {
        throw std::runtime_error("failed to open ELF for entry lookup: " + elf_path);
    }

    uint8_t hdr[32] = {};
    size_t read_count = std::fread(hdr, 1, sizeof(hdr), elf);
    std::fclose(elf);
    if (read_count != sizeof(hdr) || hdr[0] != 0x7f || hdr[1] != 'E' ||
        hdr[2] != 'L' || hdr[3] != 'F' || hdr[5] != 1) {
        throw std::runtime_error("unsupported ELF header: " + elf_path);
    }

    if (hdr[4] == 1) {
        return read_le_value(&hdr[24], 4);
    }
    if (hdr[4] == 2) {
        return read_le_value(&hdr[24], 8);
    }

    throw std::runtime_error("unsupported ELF class: " + elf_path);
}
