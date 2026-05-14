#ifndef COSIM_COSIM_CONFIG_POLICY_H
#define COSIM_COSIM_CONFIG_POLICY_H

#include "riscv_simulator.h"

#include <cstdint>
#include <string>

namespace cosim {

struct CosimPolicyArgs {
    std::string cpu_name;
    std::string elf_path;
    std::string isa = "RV32IMC";
    uint64_t memory_base = 0x80000000;
    uint64_t memory_size = 128 * 1024;
    bool dtb_enabled = false;
    std::string dtb_file;
    bool sim_mmio_enabled = true;
};

CosimConfig BuildCosimConfig(const CosimPolicyArgs& args,
                             const char* target_cosim_env,
                             const char* target_commit_env,
                             const char* legacy_commit_env = nullptr);

} // namespace cosim

#endif
