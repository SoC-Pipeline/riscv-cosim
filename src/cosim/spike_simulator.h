#ifndef COSIM_SPIKE_SIMULATOR_H
#define COSIM_SPIKE_SIMULATOR_H

#include "riscv_simulator.h"

#include <sys/syscall.h>
#include <memory>

class SpikeSimulator final : public RiscvSimulator {
public:
    SpikeSimulator();
    ~SpikeSimulator() override;

    SimulatorCapabilities capabilities() const override;
    void init(const CosimConfig& config) override;
    void step(uint64_t count) override;
    uint64_t pc() const override;
    uint32_t instruction_at(uint64_t pc) override;
    uint64_t reg(unsigned index) const override;
    void set_csr(unsigned csr_num, uint32_t value) override;
    uint32_t get_csr(unsigned csr_num) override;
    void set_mip(uint32_t mip) override;
    void set_nmi(bool nmi) override;
    void set_debug_req(bool debug_req) override;
    bool finished() const override;
    const char* name() const override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

#endif
