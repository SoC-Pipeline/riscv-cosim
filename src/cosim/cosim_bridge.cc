#include "cosim_bridge.h"

#include "cosim_session.h"

#include <exception>
#include <memory>
#include <string>
#include <vector>

namespace {

std::unique_ptr<CosimSession> g_session;

std::string cstr_or_empty(const char* s)
{
    return s ? std::string(s) : std::string();
}

} // namespace

extern "C" {

int cosim_bridge_init(const CosimConfig* config)
{
    if (!config) {
        return -1;
    }
    try {
        g_session = std::make_unique<CosimSession>();
        g_session->init(*config);
        return 0;
    } catch (const std::exception&) {
        g_session.reset();
        return -2;
    }
}

int cosim_bridge_init_default(const char* elf_path, const char* isa,
                              uint64_t memory_base, uint64_t memory_size,
                              const char* log_path, const char* commit_log_path)
{
    CosimConfig config;
    config.elf_path = cstr_or_empty(elf_path);
    if (isa && isa[0] != '\0') {
        config.isa = isa;
    }
    config.memory_base = memory_base;
    config.memory_size = memory_size;
    config.log_path = cstr_or_empty(log_path);
    config.commit_log_path = cstr_or_empty(commit_log_path);
    return cosim_bridge_init(&config);
}

int cosim_bridge_initialized()
{
    return (g_session && g_session->initialized()) ? 1 : 0;
}

int cosim_bridge_retire(uint32_t pc, uint32_t instr)
{
    if (!g_session || !g_session->initialized()) {
        return -1;
    }
    try {
        g_session->retire(pc, instr);
        return 0;
    } catch (const std::exception&) {
        return -2;
    }
}

int cosim_bridge_step_detail(uint32_t write_reg, uint32_t write_reg_data, uint32_t pc,
                             int sync_trap, int suppress_reg_write)
{
    if (!g_session || !g_session->initialized()) {
        return -1;
    }
    return g_session->step_detail(write_reg, write_reg_data, pc, sync_trap != 0,
                                  suppress_reg_write != 0)
               ? 0
               : -2;
}

void cosim_bridge_set_mip(uint32_t pre_mip, uint32_t post_mip)
{
    if (!g_session || !g_session->initialized()) {
        return;
    }
    g_session->set_mip(pre_mip, post_mip);
}

void cosim_bridge_set_nmi(int nmi)
{
    if (!g_session || !g_session->initialized()) {
        return;
    }
    g_session->set_nmi(nmi != 0);
}

void cosim_bridge_set_nmi_int(int nmi_int)
{
    if (!g_session || !g_session->initialized()) {
        return;
    }
    g_session->set_nmi_int(nmi_int != 0);
}

void cosim_bridge_set_debug_req(int debug_req)
{
    if (!g_session || !g_session->initialized()) {
        return;
    }
    g_session->set_debug_req(debug_req != 0);
}

void cosim_bridge_set_mcycle(uint64_t mcycle)
{
    if (!g_session || !g_session->initialized()) {
        return;
    }
    g_session->set_mcycle(mcycle);
}

void cosim_bridge_set_csr(unsigned csr_num, uint32_t value)
{
    if (!g_session || !g_session->initialized()) {
        return;
    }
    g_session->set_csr(csr_num, value);
}

void cosim_bridge_set_ic_scr_key_valid(int valid)
{
    if (!g_session || !g_session->initialized()) {
        return;
    }
    g_session->set_ic_scr_key_valid(valid != 0);
}

void cosim_bridge_notify_dside_access(int store, uint32_t data, uint32_t addr, uint32_t be,
                                      int error, int misaligned_first, int misaligned_second,
                                      int misaligned_first_saw_error, int m_mode_access)
{
    if (!g_session || !g_session->initialized()) {
        return;
    }
    g_session->notify_dside_access(CosimDsideAccessInfo{
        .store = store != 0,
        .data = data,
        .addr = addr,
        .be = be,
        .error = error != 0,
        .misaligned_first = misaligned_first != 0,
        .misaligned_second = misaligned_second != 0,
        .misaligned_first_saw_error = misaligned_first_saw_error != 0,
        .m_mode_access = m_mode_access != 0
    });
}

void cosim_bridge_set_iside_error(uint32_t addr)
{
    if (!g_session || !g_session->initialized()) {
        return;
    }
    g_session->set_iside_error(addr);
}

uint32_t cosim_bridge_error_count()
{
    if (!g_session || !g_session->initialized()) {
        return 0;
    }
    return static_cast<uint32_t>(g_session->get_errors().size());
}

const char* cosim_bridge_error_at(uint32_t index)
{
    if (!g_session || !g_session->initialized()) {
        return "";
    }
    const std::vector<std::string>& errors = g_session->get_errors();
    if (index >= errors.size()) {
        return "";
    }
    return errors[index].c_str();
}

void cosim_bridge_clear_errors()
{
    if (!g_session || !g_session->initialized()) {
        return;
    }
    g_session->clear_errors();
}

uint32_t cosim_bridge_insn_count()
{
    if (!g_session || !g_session->initialized()) {
        return 0;
    }
    return g_session->get_insn_cnt();
}

int cosim_bridge_finish()
{
    if (!g_session) {
        return 0;
    }
    try {
        g_session->finish();
        return 0;
    } catch (const std::exception&) {
        return -1;
    }
}

uint64_t cosim_bridge_pass_count()
{
    return g_session ? g_session->pass_count() : 0;
}

uint64_t cosim_bridge_fail_count()
{
    return g_session ? g_session->fail_count() : 0;
}

void cosim_bridge_reset()
{
    g_session.reset();
}

} // extern "C"
