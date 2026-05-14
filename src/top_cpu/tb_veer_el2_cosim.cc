#include "cosim_bridge.h"
#include "cosim_config_policy.h"
#include "cosim_top_utils.h"

#include "Vtb_top.h"
#include "verilated.h"
#include "verilated_vcd_c.h"

#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>

namespace {

vluint64_t main_time = 0;

} // namespace

double sc_time_stamp() {
    return main_time;
}

extern "C" void veer_cosim_init(const char *elf_path) {
    const CosimConfig config = cosim::BuildCosimConfig(
        cosim::CosimPolicyArgs{
            .cpu_name = "veer_el2",
            .elf_path = elf_path == nullptr ? "" : elf_path,
            .isa = top_env_string("MY_ISA", "RV32IMC"),
            .memory_base = top_env_u64("VEER_EL2_RAM_BASE", 0x80000000u),
            .memory_size = top_env_u64("VEER_EL2_RAM_SIZE", 128 * 1024u),
            .dtb_enabled = false,
            .dtb_file = top_env_string("VEER_EL2_SPIKE_DTB", ""),
            .sim_mmio_enabled = true
        },
        "VEER_EL2_COSIM_LOG", "VEER_EL2_SPIKE_COMMIT_LOG");

    if (cosim_bridge_init(&config) != 0) {
        std::cerr << "failed to initialize cosim bridge" << std::endl;
    }
}

extern "C" void veer_cosim_retire(unsigned pc, unsigned instr) {
    (void)cosim_bridge_retire(pc, instr);
}

extern "C" void veer_cosim_finish() {
    (void)cosim_bridge_finish();
}

extern "C" unsigned veer_cosim_fail_count() {
    return static_cast<unsigned>(cosim_bridge_fail_count());
}

int main(int argc, char **argv) {
    std::cout << "\nVeeR EL2 VerilatorTB: Start of sim\n" << std::endl;

    Verilated::commandArgs(argc, argv);

    auto *tb = new Vtb_top;
    const uint64_t max_cycles = top_env_u64("VEER_EL2_MAX_CYCLES", 2000000u);
    const uint64_t max_half_cycles = max_cycles * 2u + 32u;
    uint64_t half_cycles = 0;
    tb->mem_signature_begin = 0x00000000;
    tb->mem_signature_end = 0x00000000;
    tb->mem_mailbox = 0xd0580000;
    tb->lsu_bus_clk_en = 1;
    tb->i_cpu_halt_req = 0;
    tb->i_cpu_run_req = 0;
    tb->mpc_debug_halt_req = 0;
    tb->mpc_debug_run_req = 0;

    VerilatedVcdC *tfp = nullptr;
#if VM_TRACE
    if (top_env_enabled("VEER_EL2_TRACE")) {
        Verilated::traceEverOn(true);
        tfp = new VerilatedVcdC;
        tb->trace(tfp, 24);
        tfp->open("tb_veer_el2.vcd");
    }
#endif

    tb->rst_l = 0;
    for (int i = 0; i < 6; i++) {
        main_time += 5;
        tb->core_clk = !tb->core_clk;
        tb->eval();
    }
    tb->rst_l = 1;

    while (!Verilated::gotFinish() && half_cycles < max_half_cycles) {
#if VM_TRACE
        if (tfp != nullptr) {
            tfp->dump(main_time);
        }
#endif
        main_time += 5;
        tb->core_clk = !tb->core_clk;
        tb->eval();
        half_cycles++;
    }

    if (!Verilated::gotFinish()) {
        std::cerr << "\nVeeR EL2 VerilatorTB: hit max cycle count " << max_cycles << std::endl;
        (void)cosim_bridge_finish();
        delete tb;
        return EXIT_FAILURE;
    }

    tb->final();

#if VM_TRACE
    if (tfp != nullptr) {
        tfp->close();
        delete tfp;
    }
#endif

    delete tb;

    std::cout << "\nVeeR EL2 VerilatorTB: End of sim" << std::endl;
    const int rc = veer_cosim_fail_count() == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
    cosim_bridge_reset();
    return rc;
}
