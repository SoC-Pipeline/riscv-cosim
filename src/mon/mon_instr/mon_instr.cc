#include "mon_instr.h"

#include "cosim_bridge.h"

#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>

namespace {

std::unique_ptr<MonInstr> g_monitor;

const char* cstr_or_default(const char* value, const char* default_value)
{
    return value && value[0] != '\0' ? value : default_value;
}

} // namespace

MonInstr::MonInstr(std::string log_path)
    : log_path_(std::move(log_path))
{
    ensure_log_open();
}

MonInstr::~MonInstr()
{
    finish();
}

bool MonInstr::retire(const MonInstrTxn& txn)
{
    const int rc = cosim_bridge_step_detail_with_csr(
        txn.gpr.valid ? txn.gpr.addr : 0, txn.gpr.valid ? txn.gpr.data : 0, txn.pc,
        txn.trap ? 1 : 0, 0, txn.csr.valid ? 1 : 0, txn.csr.addr, txn.csr.rmask, txn.csr.rdata);
    log_retire(txn, rc);
    ++retire_count_;
    return rc == 0;
}

void MonInstr::finish()
{
    if (log_.is_open()) {
        log_ << "@RETIRE NUM = " << retire_count_ << "\n";
        log_.close();
        std::cout << "Monitor log file '" << log_path_ << "' closed." << std::endl;
    }
}

void MonInstr::ensure_log_open()
{
    log_.open(log_path_);
    if (!log_.is_open()) {
        throw std::runtime_error("failed to open " + log_path_);
    }

    std::cout << "Monitor log file '" << log_path_ << "' opened successfully." << std::endl;
    log_ << "step,order,pc,instr,trap,gpr_valid,rd,rd_wdata,mem_valid,mem_addr,mem_rmask,mem_wmask,mem_rdata,mem_wdata,csr_valid,csr_addr,csr_rmask,csr_rdata,csr_wmask,csr_wdata,result\n";
}

void MonInstr::log_retire(const MonInstrTxn& txn, int rc)
{
    if (!log_.is_open()) {
        return;
    }

    log_ << std::dec << retire_count_
         << "," << txn.order
         << ",0x" << std::hex << std::setw(8) << std::setfill('0') << txn.pc
         << ",0x" << std::setw(8) << txn.instr
         << "," << std::dec << (txn.trap ? 1 : 0)
         << "," << (txn.gpr.valid ? 1 : 0)
         << "," << txn.gpr.addr
         << ",0x" << std::hex << std::setw(8) << txn.gpr.data
         << "," << std::dec << (txn.mem.valid ? 1 : 0)
         << ",0x" << std::hex << std::setw(8) << txn.mem.addr
         << ",0x" << std::setw(1) << txn.mem.rmask
         << ",0x" << std::setw(1) << txn.mem.wmask
         << ",0x" << std::setw(8) << txn.mem.rdata
         << ",0x" << std::setw(8) << txn.mem.wdata
         << "," << std::dec << (txn.csr.valid ? 1 : 0)
         << ",0x" << std::hex << std::setw(3) << txn.csr.addr
         << ",0x" << std::setw(16) << txn.csr.rmask
         << ",0x" << std::setw(16) << txn.csr.rdata
         << ",0x" << std::setw(16) << txn.csr.wmask
         << ",0x" << std::setw(16) << txn.csr.wdata
         << "," << (rc == 0 ? "PASS" : "FAIL")
         << std::dec << std::setfill(' ') << "\n";
}

extern "C" {

void mon_instr_init(const char* log_path)
{
    g_monitor = std::make_unique<MonInstr>(cstr_or_default(log_path, "log/picorv32_mon.log"));
}

int mon_instr_retire(const MonInstrTxn* txn)
{
    if (!g_monitor || !txn) {
        return -1;
    }
    return g_monitor->retire(*txn) ? 0 : -2;
}

void mon_instr_finish()
{
    if (g_monitor) {
        g_monitor->finish();
    }
}

void mon_instr_reset()
{
    g_monitor.reset();
}

}
