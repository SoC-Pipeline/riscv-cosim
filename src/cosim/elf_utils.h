#ifndef COSIM_ELF_UTILS_H
#define COSIM_ELF_UTILS_H

#include <cstdint>
#include <string>

uint64_t read_elf_entry(const std::string& elf_path);

#endif
