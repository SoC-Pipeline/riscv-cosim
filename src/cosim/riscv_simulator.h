#ifndef COSIM_RISCV_SIMULATOR_H
#define COSIM_RISCV_SIMULATOR_H

#include <cstdint>
#include <string>

struct CosimConfig {
    std::string backend = "spike";
    std::string elf_path;
    std::string isa = "RV32IMC";
    uint64_t memory_base = 0x80000000;
    uint64_t memory_size = 128 * 1024;
    std::string log_path = "dump/cosim_result.log";
    std::string commit_log_path;
    std::string dtb_file;
    bool dtb_enabled = true;
    bool sim_mmio_enabled = false;
    uint64_t sim_print_addr = 0x10000000;
    uint64_t sim_finish_addr = 0x20000000;
    uint32_t sim_finish_value = 123456789;
};

struct SimulatorCapabilities {
    bool instruction_fetch = true;
    bool csr_access = true;
    bool interrupt_sync = true;
    bool debug_req = true;
    bool finish_detection = true;
};

class RiscvSimulator {
public:
    virtual ~RiscvSimulator() = default;

    virtual SimulatorCapabilities capabilities() const = 0;
    virtual void init(const CosimConfig& config) = 0;
    virtual void step(uint64_t count) = 0;
    virtual uint64_t pc() const = 0;
    virtual uint32_t instruction_at(uint64_t pc) = 0;
    virtual uint64_t reg(unsigned index) const = 0;
    virtual void set_csr(unsigned csr_num, uint32_t value) = 0;
    virtual uint32_t get_csr(unsigned csr_num) = 0;
    virtual void set_mip(uint32_t mip) = 0;
    virtual void set_nmi(bool nmi) = 0;
    virtual void set_debug_req(bool debug_req) = 0;
    virtual bool finished() const = 0;
    virtual const char* name() const = 0;
};

#endif
