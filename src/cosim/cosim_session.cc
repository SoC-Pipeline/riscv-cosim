#include "cosim_session.h"

#include "simulator_factory.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

CosimSession::CosimSession() = default;

CosimSession::~CosimSession()
{
    reset();
}

void CosimSession::init(const CosimConfig& config)
{
    reset();

    config_ = config;
    simulator_ = create_simulator(config_);
    simulator_->init(config_);
    initialized_ = true;

    ensure_log_open();
}

void CosimSession::retire(uint32_t dut_pc, uint32_t dut_instr)
{
    if (!initialized_ || !simulator_) {
        throw std::runtime_error("cosim is not initialized");
    }
    if (finished_) {
        throw std::runtime_error("cosim already finished");
    }
    if (reference_done_) {
        return;
    }
    if (!can_compare_retire()) {
        ++pass_count_;
        ++compare_index_;
        (void)dut_pc;
        (void)dut_instr;
        return;
    }

    uint64_t golden_pc = simulator_->pc();
    uint32_t golden_instr = simulator_->instruction_at(golden_pc);
    if (!reference_entry_valid(golden_pc, golden_instr)) {
        if (!is_ebreak(golden_instr)) {
            simulator_->step(1);
        } else {
            reference_done_ = true;
        }
        return;
    }

    bool pass = (static_cast<uint32_t>(golden_pc) == dut_pc) && (golden_instr == dut_instr);
    if (pass) {
        ++pass_count_;
        if (log_.is_open()) {
            log_ << std::left << std::setw(15) << compare_index_
                 << ", 0x" << std::right << std::hex << std::setw(8) << std::setfill('0') << static_cast<uint32_t>(golden_pc)
                 << "   0x" << std::setw(8) << golden_instr
                 << " \t  0x" << std::setw(8) << dut_pc
                 << "    0x" << std::setw(8) << dut_instr
                 << std::dec << std::setfill(' ') << "\n";
        }
    } else {
        ++fail_count_;
        std::cout << "[FAIL] [" << compare_index_ << "] Golden PC = "
                  << std::hex << std::setw(8) << std::setfill('0') << static_cast<uint32_t>(golden_pc)
                  << ", Golden Instr = " << std::setw(8) << golden_instr
                  << "  /  DUT PC = " << std::setw(8) << dut_pc
                  << ", DUT Instr = " << std::setw(8) << dut_instr
                  << std::dec << std::setfill(' ') << std::endl;
    }

    ++compare_index_;
    if (!is_ebreak(golden_instr)) {
        simulator_->step(1);
        if (simulator_->finished()) {
            reference_done_ = true;
        }
    } else {
        reference_done_ = true;
    }
}

bool CosimSession::step_detail(uint32_t write_reg, uint32_t write_reg_data, uint32_t pc,
                               bool sync_trap, bool suppress_reg_write)
{
    return step_detail_with_csr(write_reg, write_reg_data, pc, sync_trap,
                                suppress_reg_write, false, 0, 0, 0);
}

bool CosimSession::step_detail_with_csr(uint32_t write_reg, uint32_t write_reg_data, uint32_t pc,
                                        bool sync_trap, bool suppress_reg_write, bool csr_valid,
                                        unsigned csr_num, uint64_t csr_rmask, uint64_t csr_rdata)
{
    if (!initialized_ || !simulator_) {
        errors_.push_back("cosim is not initialized");
        return false;
    }

    if (finished_) {
        errors_.push_back("cosim already finished");
        return false;
    }

    auto push_error = [this](const std::string& msg) {
        errors_.push_back(msg);
        ++fail_count_;
        if (log_.is_open()) {
            log_ << "[COSIM FAIL] " << msg << "\n";
        }
    };
    const SimulatorCapabilities caps = simulator_->capabilities();
    if (!caps.csr_access && !caps.interrupt_sync && !caps.debug_req) {
        push_error("backend does not support step-detail synchronization requirements");
        return false;
    }

    uint64_t before_pc = simulator_->pc();
    uint64_t before_regs[32] = {0};
    for (unsigned i = 0; i < 32; ++i) {
        before_regs[i] = simulator_->reg(i);
    }

    try {
        simulator_->step(1);
    } catch (const std::exception& ex) {
        push_error(std::string("step failed: ") + ex.what());
        return false;
    }

    if ((before_pc & 0xffffffffu) != pc) {
        std::ostringstream oss;
        oss << "PC mismatch, DUT retired: 0x" << std::hex << pc
            << ", ISS retired: 0x" << static_cast<uint32_t>(before_pc & 0xffffffffu);
        push_error(oss.str());
        return false;
    }

    if (sync_trap && write_reg != 0) {
        std::ostringstream oss;
        oss << "sync trap at PC 0x" << std::hex << pc
            << " but DUT wrote x" << std::dec << write_reg;
        push_error(oss.str());
        return false;
    }

    unsigned changed_reg = 0;
    unsigned changed_count = 0;
    uint32_t changed_val = 0;
    for (unsigned i = 1; i < 32; ++i) {
        uint32_t old_v = static_cast<uint32_t>(before_regs[i] & 0xffffffffu);
        uint32_t new_v = static_cast<uint32_t>(simulator_->reg(i) & 0xffffffffu);
        if (old_v != new_v) {
            ++changed_count;
            changed_reg = i;
            changed_val = new_v;
        }
    }

    constexpr unsigned kCsrCycle = 0xC00;
    constexpr unsigned kCsrCycleh = 0xC80;
    constexpr unsigned kCsrInstret = 0xC02;
    constexpr unsigned kCsrInstreth = 0xC82;
    const bool volatile_counter = csr_valid &&
                                  (csr_num == kCsrCycle || csr_num == kCsrCycleh ||
                                   csr_num == kCsrInstret || csr_num == kCsrInstreth);
    const bool compare_gpr_write = !volatile_counter;

    if (!sync_trap) {
        if (suppress_reg_write) {
            if (write_reg != 0) {
                std::ostringstream oss;
                oss << "suppress_reg_write set but DUT wrote x" << std::dec << write_reg;
                push_error(oss.str());
                return false;
            }
            // In suppress mode DUT reports no write, while ISS may still update one GPR.
            if (changed_count > 1) {
                std::ostringstream oss;
                oss << "suppressed write expected <=1 GPR change, got " << changed_count;
                push_error(oss.str());
                return false;
            }
        } else if (!compare_gpr_write) {
            if (changed_count > 1) {
                std::ostringstream oss;
                oss << "volatile CSR retire expected <=1 GPR change, got " << changed_count;
                push_error(oss.str());
                return false;
            }
        } else if (write_reg == 0) {
            if (changed_count != 0) {
                std::ostringstream oss;
                oss << "DUT no GPR write but ISS wrote x" << changed_reg;
                push_error(oss.str());
                return false;
            }
        } else {
            if (changed_count == 0) {
                // Some cores may report a writeback event even when the resulting
                // value is unchanged. Accept this when the reported data matches
                // the post-step ISS register value.
                uint32_t post_val = static_cast<uint32_t>(simulator_->reg(write_reg) & 0xffffffffu);
                if (post_val != write_reg_data) {
                    std::ostringstream oss;
                    oss << "DUT wrote x" << write_reg
                        << " but ISS did not write any GPR and value mismatch"
                        << " DUT=0x" << std::hex << write_reg_data
                        << ", ISS=0x" << post_val;
                    push_error(oss.str());
                    return false;
                }
            } else if (changed_count != 1 || changed_reg != write_reg) {
                std::ostringstream oss;
                oss << "GPR write index mismatch, DUT x" << write_reg
                    << ", ISS x" << changed_reg << " (count=" << changed_count << ")";
                push_error(oss.str());
                return false;
            } else if (changed_val != write_reg_data) {
                std::ostringstream oss;
                oss << "GPR write data mismatch x" << write_reg
                    << ", DUT=0x" << std::hex << write_reg_data
                    << ", ISS=0x" << changed_val;
                push_error(oss.str());
                return false;
            }
        }
    }

    if (csr_valid && csr_rmask != 0) {
        if (!volatile_counter) {
            const uint64_t iss_csr = static_cast<uint64_t>(get_csr(csr_num));
            const uint64_t masked_dut = csr_rdata & csr_rmask;
            const uint64_t masked_iss = iss_csr & csr_rmask;
            if (masked_dut != masked_iss) {
                std::ostringstream oss;
                oss << "CSR mismatch 0x" << std::hex << csr_num
                    << ", DUT=0x" << masked_dut
                    << ", ISS=0x" << masked_iss
                    << " (mask=0x" << csr_rmask << ")";
                push_error(oss.str());
                return false;
            }
        }
    }

    ++pass_count_;
    ++compare_index_;
    if (log_.is_open()) {
        log_ << std::left << std::setw(15) << compare_index_ - 1
             << ", detail PASS pc=0x" << std::right << std::hex << std::setw(8) << std::setfill('0') << pc
             << " rd=x" << std::dec << write_reg
             << " data=0x" << std::hex << std::setw(8) << write_reg_data
             << " csr=" << (csr_valid ? "0x" : "-");
        if (csr_valid) {
            log_ << std::hex << csr_num
                 << " mask=0x" << csr_rmask
                 << " data=0x" << csr_rdata;
        }
        log_
             << std::dec << std::setfill(' ') << "\n";
    }
    return true;
}

void CosimSession::set_csr(unsigned csr_num, uint32_t value)
{
    if (!simulator_ || !simulator_->capabilities().csr_access) {
        return;
    }
    constexpr unsigned kCsrMstatus = 0x300;
    constexpr unsigned kCsrMisa = 0x301;
    constexpr unsigned kCsrMtvec = 0x305;
    constexpr unsigned kCsrMcause = 0x342;

    if (csr_num == kCsrMstatus) {
        constexpr uint32_t kMstatusMask = (1u << 3) | (1u << 7) | (1u << 17) | (3u << 11) | (1u << 21);
        simulator_->set_csr(csr_num, value & kMstatusMask);
        return;
    }

    if (csr_num == kCsrMcause) {
        const uint32_t any_interrupt = value & 0x80000000u;
        const uint32_t int_interrupt = value & 0x40000000u;
        uint32_t new_val = (value & 0x1fu) | any_interrupt;
        if (any_interrupt && int_interrupt) {
            new_val |= 0x7fffffe0u;
        }
        simulator_->set_csr(csr_num, new_val);
        return;
    }

    if (csr_num == kCsrMtvec) {
        simulator_->set_csr(csr_num, (value & 0xffffff00u) | 0x1u);
        return;
    }

    if (csr_num == kCsrMisa) {
        simulator_->set_csr(csr_num, 0x40901104u);
        return;
    }

    simulator_->set_csr(csr_num, value);
}

uint32_t CosimSession::get_csr(unsigned csr_num) const
{
    if (!simulator_) {
        return 0;
    }
    return const_cast<RiscvSimulator*>(simulator_.get())->get_csr(csr_num);
}

void CosimSession::set_mip(uint32_t pre_mip, uint32_t post_mip)
{
    (void)post_mip;
    if (!simulator_ || !simulator_->capabilities().interrupt_sync) {
        return;
    }
    simulator_->set_mip(pre_mip);
}

void CosimSession::set_nmi(bool nmi)
{
    if (!simulator_ || !simulator_->capabilities().interrupt_sync) {
        return;
    }
    simulator_->set_nmi(nmi);
}

void CosimSession::set_nmi_int(bool nmi_int)
{
    set_nmi(nmi_int);
}

void CosimSession::set_debug_req(bool debug_req)
{
    if (!simulator_ || !simulator_->capabilities().debug_req) {
        return;
    }
    simulator_->set_debug_req(debug_req);
}

void CosimSession::set_mcycle(uint64_t mcycle)
{
    set_csr(0xB00, static_cast<uint32_t>(mcycle & 0xffffffffu));
    set_csr(0xB80, static_cast<uint32_t>(mcycle >> 32));
}

void CosimSession::set_ic_scr_key_valid(bool valid)
{
    (void)valid;
}

void CosimSession::notify_dside_access(const CosimDsideAccessInfo& access_info)
{
    (void)access_info;
}

void CosimSession::set_iside_error(uint32_t addr)
{
    (void)addr;
}

const std::vector<std::string>& CosimSession::get_errors() const
{
    return errors_;
}

void CosimSession::clear_errors()
{
    errors_.clear();
}

unsigned CosimSession::get_insn_cnt() const
{
    return static_cast<unsigned>(pass_count_);
}

void CosimSession::finish()
{
    if (!initialized_ || finished_) {
        return;
    }

    std::cout << "COMPARE PASS NUM = " << pass_count_
              << ", COMPARE FAIL NUM = " << fail_count_ << std::endl;

    if (log_.is_open()) {
        log_ << "@PASS NUM = " << pass_count_ << "\n@FAIL NUM = " << fail_count_ << "\n";
        log_.close();
        std::cout << "Log file '" << config_.log_path << "' closed." << std::endl;
    }

    finished_ = true;
}

void CosimSession::reset()
{
    if (initialized_ && !finished_) {
        finish();
    }
    if (log_.is_open()) {
        log_.close();
    }
    simulator_.reset();
    compare_index_ = 0;
    pass_count_ = 0;
    fail_count_ = 0;
    initialized_ = false;
    finished_ = false;
    reference_done_ = false;
    errors_.clear();
}

uint64_t CosimSession::pass_count() const
{
    return pass_count_;
}

uint64_t CosimSession::fail_count() const
{
    return fail_count_;
}

bool CosimSession::initialized() const
{
    return initialized_;
}

bool CosimSession::reference_entry_valid(uint64_t pc, uint32_t instr) const
{
    uint32_t pc32 = static_cast<uint32_t>(pc);
    uint32_t memory_base = static_cast<uint32_t>(config_.memory_base);
    uint32_t memory_end = memory_base + static_cast<uint32_t>(config_.memory_size);
    return pc32 >= memory_base && pc32 < memory_end && instr != 0xffffffff;
}

bool CosimSession::is_ebreak(uint32_t instr) const
{
    return instr == 0x00009002 || instr == 0x00100073;
}

bool CosimSession::can_compare_retire() const
{
    return simulator_ && simulator_->capabilities().instruction_fetch;
}

void CosimSession::ensure_log_open()
{
    const std::string log_path = config_.log_path.empty() ? "dump/cosim_result.log" : config_.log_path;
    log_.open(log_path);
    if (!log_.is_open()) {
        throw std::runtime_error("failed to open " + log_path);
    }

    std::cout << "Log file '" << log_path << "' opened successfully." << std::endl;
    log_ << "[Step Num],     [Golden PC]  [Golden Instr]    [DuT PC]  [DuT Instr]\n";
}
