// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <svdpi.h>

#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include <sys/stat.h>
#include <sys/types.h>

#include "Vtb_ibex__Syms.h"
#include "cosim_bridge.h"
#include "cosim_config_policy.h"
#include "cosim_top_utils.h"
#include "cosim.h"
#include "ibex_pcounts.h"
#include "mon_instr.h"
#include "verilated_toplevel.h"
#include "verilator_memutil.h"
#include "verilator_sim_ctrl.h"

namespace {

constexpr uint32_t kRamSizeBytes = 0x00100000u;

class TbIbexCosim {
 public:
  TbIbexCosim()
      : memutil_(),
        ram_("TOP.tb_ibex.u_ram.u_ram", kRamSizeBytes / 4, 4),
        ram_base_(top_env_u32("IBEX_RAM_BASE", 0x80000000u)),
        reset_vector_(top_env_u32("IBEX_RESET_VECTOR", 0x80000080u)),
        cosim_(&cosim_bridge_adapter_) {}

  int Main(int argc, char **argv) {
    bool exit_app;
    int ret_code = Setup(argc, argv, exit_app);

    if (exit_app) {
      return ret_code;
    }

    Run();

    if (!Finish()) {
      return 1;
    }

    return 0;
  }

  void CreateCosim(bool secure_ibex, bool icache_en,
                   uint32_t pmp_num_regions,
                   uint32_t pmp_granularity,
                   uint32_t mhpm_counter_num,
                   uint32_t dm_start_addr,
                   uint32_t dm_end_addr) {
    (void)secure_ibex;
    (void)icache_en;
    (void)pmp_num_regions;
    (void)pmp_granularity;
    (void)mhpm_counter_num;
    (void)dm_start_addr;
    (void)dm_end_addr;
    const CosimConfig config = cosim::BuildCosimConfig(
        cosim::CosimPolicyArgs{
            .cpu_name = "ibex",
            .elf_path = GetElfPath(),
            .isa = GetIsaString(),
            .memory_base = ram_base_,
            .memory_size = kRamSizeBytes,
            .dtb_enabled = false,
            .sim_mmio_enabled = true
        },
        "IBEX_COSIM_LOG", "SPIKE_COMMIT_LOG", "IBEX_SPIKE_COMMIT_LOG");

    if (cosim_bridge_init(&config) != 0) {
      throw std::runtime_error("failed to initialize cosim bridge");
    }
  }

  Cosim *GetCosim() {
    assert(cosim_);
    return cosim_;
  }

 private:
  tb_ibex top_;
  VerilatorMemUtil memutil_;
  MemArea ram_;
  uint32_t ram_base_;
  uint32_t reset_vector_;
  struct CosimBridgeAdapter : public Cosim {
#define COSIM_BRIDGE_VOID_FORWARD(method, fn, decl, call) \
    void method decl override { fn call; }
#define COSIM_BRIDGE_BOOL_FORWARD(method, fn, decl, call) \
    bool method decl override { return fn call == 0; }

    void add_memory(uint32_t base_addr, size_t size) override { (void)base_addr; (void)size; }
    bool backdoor_write_mem(uint32_t addr, size_t len, const uint8_t *data_in) override
    { (void)addr; (void)len; (void)data_in; return true; }
    bool backdoor_read_mem(uint32_t addr, size_t len, uint8_t *data_out) override
    { (void)addr; (void)len; (void)data_out; return true; }
    COSIM_BRIDGE_BOOL_FORWARD(step, cosim_bridge_step_detail,
                              (uint32_t write_reg, uint32_t write_reg_data, uint32_t pc, bool sync_trap, bool suppress_reg_write),
                              (write_reg, write_reg_data, pc, sync_trap, suppress_reg_write))
    COSIM_BRIDGE_VOID_FORWARD(set_mip, cosim_bridge_set_mip,
                              (uint32_t pre_mip, uint32_t post_mip),
                              (pre_mip, post_mip))
    COSIM_BRIDGE_VOID_FORWARD(set_nmi, cosim_bridge_set_nmi, (bool nmi), (nmi))
    COSIM_BRIDGE_VOID_FORWARD(set_nmi_int, cosim_bridge_set_nmi_int, (bool nmi_int), (nmi_int))
    COSIM_BRIDGE_VOID_FORWARD(set_debug_req, cosim_bridge_set_debug_req, (bool debug_req), (debug_req))
    COSIM_BRIDGE_VOID_FORWARD(set_mcycle, cosim_bridge_set_mcycle, (uint64_t mcycle), (mcycle))
    void set_csr(const int csr_num, const uint32_t new_val) override {
      cosim_bridge_set_csr(static_cast<unsigned>(csr_num), new_val);
    }
    COSIM_BRIDGE_VOID_FORWARD(set_ic_scr_key_valid, cosim_bridge_set_ic_scr_key_valid, (bool valid), (valid))
    void notify_dside_access(const DSideAccessInfo &access_info) override {
      cosim_bridge_notify_dside_access(access_info.store, access_info.data, access_info.addr,
                                       access_info.be, access_info.error,
                                       access_info.misaligned_first,
                                       access_info.misaligned_second,
                                       access_info.misaligned_first_saw_error,
                                       access_info.m_mode_access);
    }
    void set_iside_error(uint32_t addr) override { cosim_bridge_set_iside_error(addr); }
    const std::vector<std::string> &get_errors() override {
      errors_cache_.clear();
      const uint32_t count = cosim_bridge_error_count();
      errors_cache_.reserve(count);
      for (uint32_t i = 0; i < count; ++i) {
        errors_cache_.emplace_back(cosim_bridge_error_at(i));
      }
      return errors_cache_;
    }
    void clear_errors() override { cosim_bridge_clear_errors(); }
    unsigned int get_insn_cnt() override { return cosim_bridge_insn_count(); }
#undef COSIM_BRIDGE_BOOL_FORWARD
#undef COSIM_BRIDGE_VOID_FORWARD
   private:
    std::vector<std::string> errors_cache_;
  };

  Cosim *cosim_;
  CosimBridgeAdapter cosim_bridge_adapter_;

  std::string GetIsaString() const {
    const Vtb_ibex &top = top_;
    assert(top.tb_ibex);

    std::string base = top.tb_ibex->RV32E ? "rv32e" : "rv32i";
    std::string extensions;

    if (top.tb_ibex->RV32M) {
      extensions += "m";
    }

    extensions += "c";

    switch (top.tb_ibex->RV32B) {
      case 0:
        break;
      case 1:
        extensions += "_Zba_Zbb_Zbs_XZbf_XZbt";
        break;
      case 2:
        extensions += "_Zba_Zbb_Zbc_Zbs_XZbf_XZbp_XZbr_XZbt";
        break;
      case 3:
        extensions += "_Zba_Zbb_Zbc_Zbs_XZbe_XZbf_XZbp_XZbr_XZbt";
        break;
    }

    return base + extensions;
  }

  std::string GetElfPath() const {
    return top_env_string("TEST_ELF", "build/firmware/hello/obj/firmware.elf");
  }

  std::string GetPcountCsvPath() const {
    return top_env_string("IBEX_PCOUNT_CSV", "log/tb_ibex_pcount.csv");
  }

  void CopyMemAreaToCosim(MemArea *area, uint32_t base_addr) {
    (void)area;
    (void)base_addr;
  }

  int Setup(int argc, char **argv, bool &exit_app) {
    VerilatorSimCtrl &simctrl = VerilatorSimCtrl::GetInstance();

    simctrl.SetTop(&top_, &top_.IO_CLK, &top_.IO_RST_N,
                   VerilatorSimCtrlFlags::ResetPolarityNegative);

    memutil_.RegisterMemoryArea("ram", ram_base_, &ram_);
    simctrl.RegisterExtension(&memutil_);

    exit_app = false;
    return simctrl.ParseCommandArgs(argc, argv, exit_app);
  }

  void Run() {
    VerilatorSimCtrl &simctrl = VerilatorSimCtrl::GetInstance();

    top_ensure_directory("log");

    std::cout << "Simulation of Ibex tb" << std::endl
              << "=====================" << std::endl
              << std::endl;

    simctrl.RunSimulation();
  }

  bool Finish() {
    VerilatorSimCtrl &simctrl = VerilatorSimCtrl::GetInstance();

    if (!simctrl.WasSimulationSuccessful()) {
      return false;
    }

    svSetScope(svGetScopeFromName("TOP.tb_ibex"));

    std::cout << "\nPerformance Counters" << std::endl
              << "====================" << std::endl;
    std::cout << ibex_pcount_string(false);

    const std::string pcount_csv_path = GetPcountCsvPath();
    top_ensure_parent_directory(pcount_csv_path);
    std::ofstream pcount_csv(pcount_csv_path);
    if (!pcount_csv.is_open()) {
      throw std::runtime_error("failed to open " + pcount_csv_path);
    }
    pcount_csv << ibex_pcount_string(true);

    std::cout << "Co-simulation matched " << cosim_->get_insn_cnt()
              << " instructions\n";

    cosim_bridge_reset();
    return true;
  }
};

TbIbexCosim *tb_ibex_cosim;

}  // namespace

extern "C" {
void *get_spike_cosim() {
  assert(tb_ibex_cosim);
  return static_cast<Cosim *>(tb_ibex_cosim->GetCosim());
}

void create_cosim(svBit secure_ibex, svBit icache_en,
                  const svBitVecVal *pmp_num_regions,
                  const svBitVecVal *pmp_granularity,
                  const svBitVecVal *mhpm_counter_num,
                  const svBitVecVal *dm_start_addr,
                  const svBitVecVal *dm_end_addr) {
  assert(tb_ibex_cosim);
  tb_ibex_cosim->CreateCosim(secure_ibex, icache_en, pmp_num_regions[0],
                             pmp_granularity[0], mhpm_counter_num[0],
                             dm_start_addr[0], dm_end_addr[0]);
}

void ibex_mon_init() {
  const std::string mon_log_path =
      top_env_string("IBEX_MON_LOG", "log/ibex_mon.log");
  top_ensure_parent_directory(mon_log_path);
  mon_instr_init_mode(mon_log_path.c_str(),
                      static_cast<uint32_t>(MonInstrCompareMode::LogOnly));
}

int ibex_mon_retire(uint32_t order, uint32_t pc, uint32_t instr, svBit trap,
                    svBit gpr_valid, uint32_t rd_addr, uint32_t rd_wdata) {
  return mon_instr_retire_simple(order, pc, instr, trap != 0, gpr_valid != 0,
                                 rd_addr, rd_wdata);
}

void ibex_mon_finish() {
  mon_instr_finish();
}
}

int main(int argc, char **argv) {
  tb_ibex_cosim = new TbIbexCosim();

  int ret_code = tb_ibex_cosim->Main(argc, argv);

  mon_instr_reset();
  delete tb_ibex_cosim;
  tb_ibex_cosim = nullptr;

  return ret_code;
}
