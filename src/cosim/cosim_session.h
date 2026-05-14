#ifndef COSIM_COSIM_SESSION_H
#define COSIM_COSIM_SESSION_H

#include "riscv_simulator.h"

#include <cstdint>
#include <sys/syscall.h>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

struct CosimDsideAccessInfo {
    bool store;
    uint32_t data;
    uint32_t addr;
    uint32_t be;
    bool error;
    bool misaligned_first;
    bool misaligned_second;
    bool misaligned_first_saw_error;
    bool m_mode_access;
};

class CosimSession {
public:
    CosimSession();
    ~CosimSession();

    void init(const CosimConfig& config);
    void retire(uint32_t dut_pc, uint32_t dut_instr);
    bool step_detail(uint32_t write_reg, uint32_t write_reg_data, uint32_t pc,
                     bool sync_trap, bool suppress_reg_write);
    void set_csr(unsigned csr_num, uint32_t value);
    uint32_t get_csr(unsigned csr_num) const;
    void set_mip(uint32_t pre_mip, uint32_t post_mip);
    void set_nmi(bool nmi);
    void set_nmi_int(bool nmi_int);
    void set_debug_req(bool debug_req);
    void set_mcycle(uint64_t mcycle);
    void set_ic_scr_key_valid(bool valid);
    void notify_dside_access(const CosimDsideAccessInfo& access_info);
    void set_iside_error(uint32_t addr);
    const std::vector<std::string>& get_errors() const;
    void clear_errors();
    unsigned get_insn_cnt() const;
    void finish();
    void reset();

    uint64_t pass_count() const;
    uint64_t fail_count() const;
    bool initialized() const;

private:
    bool reference_entry_valid(uint64_t pc, uint32_t instr) const;
    bool is_ebreak(uint32_t instr) const;
    bool can_compare_retire() const;
    void ensure_log_open();

    std::unique_ptr<RiscvSimulator> simulator_;
    CosimConfig config_;
    std::ofstream log_;
    uint64_t compare_index_ = 0;
    uint64_t pass_count_ = 0;
    uint64_t fail_count_ = 0;
    bool initialized_ = false;
    bool finished_ = false;
    bool reference_done_ = false;
    std::vector<std::string> errors_;
};

#endif
