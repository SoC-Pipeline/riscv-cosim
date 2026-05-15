#include "Vtb_picorv32.h"
#include "cosim_bridge.h"
#include "cosim_config_policy.h"
#include "cosim_top_utils.h"
#include "mon_instr.h"
#include "verilated.h"
#include "verilated_vcd_c.h"

#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

vluint64_t main_time = 0;
bool test_passed = false;

void EnsureParentDirectory(const std::string& path)
{
    const std::size_t slash = path.find_last_of('/');
    if (slash == std::string::npos) {
        return;
    }
    top_ensure_directory(path.substr(0, slash));
}

std::string GetElfPath(const char* elf_path)
{
    if (elf_path != nullptr && elf_path[0] != '\0') {
        return elf_path;
    }
    return top_env_string("TEST_ELF", "build/firmware/hello/obj/firmware.elf");
}

} // namespace

double sc_time_stamp()
{
    return static_cast<double>(main_time);
}

extern "C" int picorv32_cosim_init(const char* elf_path)
{
    try {
        const std::string resolved_elf = GetElfPath(elf_path);
        const CosimConfig config = cosim::BuildCosimConfig(
            cosim::CosimPolicyArgs{
                .cpu_name = "picorv32",
                .elf_path = resolved_elf,
                .isa = top_env_string("MY_ISA", "RV32IMC"),
                .memory_base = 0x80000000u,
                .memory_size = 128 * 1024u,
                .dtb_enabled = false,
                .sim_mmio_enabled = true
            },
            "PICORV32_COSIM_LOG", "PICORV32_SPIKE_COMMIT_LOG");

        if (cosim_bridge_init(&config) != 0) {
            std::cerr << "failed to initialize PicoRV32 cosim bridge" << std::endl;
            return -1;
        }

        const std::string mon_log_path =
            top_env_string("PICORV32_MON_LOG", "log/picorv32_mon.log");
        EnsureParentDirectory(mon_log_path);
        mon_instr_init(mon_log_path.c_str());
        test_passed = false;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "PicoRV32 cosim init exception: " << e.what() << std::endl;
        mon_instr_reset();
        cosim_bridge_reset();
        return -1;
    }
}

extern "C" int picorv32_cosim_monitor_retire(
    unsigned order,
    unsigned pc,
    unsigned instr,
    unsigned trap,
    unsigned rd_addr,
    unsigned rd_wdata,
    unsigned mem_addr,
    unsigned mem_rmask,
    unsigned mem_wmask,
    unsigned mem_rdata,
    unsigned mem_wdata,
    unsigned csr_addr,
    unsigned csr_rmask_lo,
    unsigned csr_rmask_hi,
    unsigned csr_rdata_lo,
    unsigned csr_rdata_hi,
    unsigned csr_wmask_lo,
    unsigned csr_wmask_hi,
    unsigned csr_wdata_lo,
    unsigned csr_wdata_hi)
{
    MonInstrTxn txn;
    txn.order = order;
    txn.pc = pc;
    txn.instr = instr;
    txn.trap = trap != 0;
    txn.gpr.valid = rd_addr != 0 || rd_wdata != 0;
    txn.gpr.addr = rd_addr;
    txn.gpr.data = rd_wdata;
    txn.mem.valid = mem_rmask != 0 || mem_wmask != 0;
    txn.mem.addr = mem_addr;
    txn.mem.rmask = mem_rmask;
    txn.mem.wmask = mem_wmask;
    txn.mem.rdata = mem_rdata;
    txn.mem.wdata = mem_wdata;
    txn.csr.valid = csr_rmask_lo != 0 || csr_rmask_hi != 0 ||
                    csr_wmask_lo != 0 || csr_wmask_hi != 0;
    txn.csr.addr = csr_addr;
    txn.csr.rmask = static_cast<uint64_t>(csr_rmask_lo) |
                    (static_cast<uint64_t>(csr_rmask_hi) << 32);
    txn.csr.rdata = static_cast<uint64_t>(csr_rdata_lo) |
                    (static_cast<uint64_t>(csr_rdata_hi) << 32);
    txn.csr.wmask = static_cast<uint64_t>(csr_wmask_lo) |
                    (static_cast<uint64_t>(csr_wmask_hi) << 32);
    txn.csr.wdata = static_cast<uint64_t>(csr_wdata_lo) |
                    (static_cast<uint64_t>(csr_wdata_hi) << 32);
    return mon_instr_retire(&txn);
}

extern "C" void picorv32_cosim_finish()
{
    mon_instr_finish();
    (void)cosim_bridge_finish();
}

extern "C" unsigned picorv32_cosim_fail_count()
{
    return static_cast<unsigned>(cosim_bridge_fail_count());
}

extern "C" void picorv32_cosim_set_test_result(unsigned char passed)
{
    test_passed = passed != 0;
}

int main(int argc, char** argv)
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::setvbuf(stderr, nullptr, _IONBF, 0);
    Verilated::commandArgs(argc, argv);

    Vtb_picorv32 top;
    const uint64_t max_time_ps =
        top_env_u64("PICORV32_MAX_TIME_PS", 0u);
    bool external_timeout = false;
    bool no_pending_events = false;

    VerilatedVcdC* tfp = nullptr;
#if VM_TRACE
    if (Verilated::commandArgsPlusMatch("vcd") &&
        std::strcmp(Verilated::commandArgsPlusMatch("vcd"), "+vcd") == 0) {
        Verilated::traceEverOn(true);
        tfp = new VerilatedVcdC;
        top.trace(tfp, 99);
        tfp->open("dump/tb_picorv32.vcd");
    }
#endif

    while (!Verilated::gotFinish()) {
        top.eval();
#if VM_TRACE
        if (tfp != nullptr) {
            tfp->dump(main_time);
        }
#endif

        if (Verilated::gotFinish()) {
            break;
        }

        if (!top.eventsPending()) {
            no_pending_events = true;
            std::cerr << "FAIL: PicoRV32 Verilator simulation has no pending events before finish." << std::endl;
            break;
        }

        const vluint64_t next_time = top.nextTimeSlot();
        if (max_time_ps != 0 && next_time > max_time_ps) {
            external_timeout = true;
            std::cerr << "FAIL: PicoRV32 Verilator simulation hit external timeout at "
                      << next_time << " ps" << std::endl;
            break;
        }

        Verilated::timeInc(next_time - main_time);
        main_time = next_time;
    }

    if (external_timeout) {
        picorv32_cosim_set_test_result(0);
    }

    top.final();

#if VM_TRACE
    if (tfp != nullptr) {
        tfp->close();
        delete tfp;
    }
#endif

    const bool success = !external_timeout &&
                         !no_pending_events &&
                         test_passed &&
                         picorv32_cosim_fail_count() == 0;
    mon_instr_reset();
    cosim_bridge_reset();
    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
