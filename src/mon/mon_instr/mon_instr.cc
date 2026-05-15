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

MonInstrCompareMode compare_mode_from_u32(uint32_t mode)
{
    switch (mode) {
    case static_cast<uint32_t>(MonInstrCompareMode::Retire):
        return MonInstrCompareMode::Retire;
    case static_cast<uint32_t>(MonInstrCompareMode::LogOnly):
        return MonInstrCompareMode::LogOnly;
    case static_cast<uint32_t>(MonInstrCompareMode::Detail):
    default:
        return MonInstrCompareMode::Detail;
    }
}

} // namespace

MonInstr::MonInstr(std::string log_path, MonInstrCompareMode compare_mode)
    : log_path_(std::move(log_path)), compare_mode_(compare_mode)
{
    ensure_log_open();
}

MonInstr::~MonInstr()
{
    finish();
}

bool MonInstr::retire(const MonInstrTxn& txn)
{
    int rc = 0;
    switch (compare_mode_) {
    case MonInstrCompareMode::Detail:
        rc = cosim_bridge_step_detail_with_csr(
            txn.gpr.valid ? txn.gpr.addr : 0, txn.gpr.valid ? txn.gpr.data : 0, txn.pc,
            txn.trap ? 1 : 0, 0, txn.csr.valid ? 1 : 0, txn.csr.addr, txn.csr.rmask, txn.csr.rdata);
        break;
    case MonInstrCompareMode::Retire:
        rc = cosim_bridge_retire(txn.pc, txn.instr);
        break;
    case MonInstrCompareMode::LogOnly:
        rc = 0;
        break;
    }
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

    const char* result = compare_mode_ == MonInstrCompareMode::LogOnly ? "LOG" :
                         (rc == 0 ? "PASS" : "FAIL");

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
         << "," << result
         << std::dec << std::setfill(' ') << "\n";
}

extern "C" {

void mon_instr_init(const char* log_path)
{
    mon_instr_init_mode(log_path, static_cast<uint32_t>(MonInstrCompareMode::Detail));
}

void mon_instr_init_mode(const char* log_path, uint32_t compare_mode)
{
    g_monitor = std::make_unique<MonInstr>(
        cstr_or_default(log_path, "log/picorv32_mon.log"),
        compare_mode_from_u32(compare_mode));
}

int mon_instr_retire(const MonInstrTxn* txn)
{
    if (!g_monitor || !txn) {
        return -1;
    }
    return g_monitor->retire(*txn) ? 0 : -2;
}

int mon_instr_retire_simple(uint32_t order, uint32_t pc, uint32_t instr, int trap,
                            int gpr_valid, uint32_t rd_addr, uint32_t rd_wdata)
{
    MonInstrTxn txn;
    txn.order = order;
    txn.pc = pc;
    txn.instr = instr;
    txn.trap = trap != 0;
    txn.gpr.valid = gpr_valid != 0;
    txn.gpr.addr = rd_addr;
    txn.gpr.data = rd_wdata;
    return mon_instr_retire(&txn);
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
