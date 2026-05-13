#ifndef COSIM_BRIDGE_H
#define COSIM_BRIDGE_H

#include <cstdint>

#include "riscv_simulator.h"

extern "C" {

int cosim_bridge_init(const CosimConfig* config);
int cosim_bridge_init_default(const char* elf_path, const char* isa,
                              uint64_t memory_base, uint64_t memory_size,
                              const char* log_path, const char* commit_log_path);
int cosim_bridge_initialized();
int cosim_bridge_retire(uint32_t pc, uint32_t instr);
int cosim_bridge_step_detail(uint32_t write_reg, uint32_t write_reg_data, uint32_t pc,
                             int sync_trap, int suppress_reg_write);
void cosim_bridge_set_mip(uint32_t pre_mip, uint32_t post_mip);
void cosim_bridge_set_nmi(int nmi);
void cosim_bridge_set_nmi_int(int nmi_int);
void cosim_bridge_set_debug_req(int debug_req);
void cosim_bridge_set_mcycle(uint64_t mcycle);
void cosim_bridge_set_csr(unsigned csr_num, uint32_t value);
void cosim_bridge_set_ic_scr_key_valid(int valid);
void cosim_bridge_notify_dside_access(int store, uint32_t data, uint32_t addr, uint32_t be,
                                      int error, int misaligned_first, int misaligned_second,
                                      int misaligned_first_saw_error, int m_mode_access);
void cosim_bridge_set_iside_error(uint32_t addr);
uint32_t cosim_bridge_error_count();
const char* cosim_bridge_error_at(uint32_t index);
void cosim_bridge_clear_errors();
uint32_t cosim_bridge_insn_count();
int cosim_bridge_finish();
uint64_t cosim_bridge_pass_count();
uint64_t cosim_bridge_fail_count();
void cosim_bridge_reset();

}

#endif
