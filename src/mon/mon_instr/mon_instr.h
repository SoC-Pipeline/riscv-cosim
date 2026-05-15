#ifndef MON_MON_INSTR_MON_INSTR_H
#define MON_MON_INSTR_MON_INSTR_H

#include "txn_instr.h"

#include <cstdint>
#include <fstream>
#include <string>

enum class MonInstrCompareMode : uint32_t {
    Detail = 0,
    Retire = 1,
    LogOnly = 2,
};

class MonInstr {
public:
    explicit MonInstr(std::string log_path, MonInstrCompareMode compare_mode);
    ~MonInstr();

    bool retire(const MonInstrTxn& txn);
    void finish();

private:
    void ensure_log_open();
    void log_retire(const MonInstrTxn& txn, int rc);

    std::string log_path_;
    MonInstrCompareMode compare_mode_ = MonInstrCompareMode::Detail;
    std::ofstream log_;
    uint64_t retire_count_ = 0;
};

extern "C" {

void mon_instr_init(const char* log_path);
void mon_instr_init_mode(const char* log_path, uint32_t compare_mode);
int mon_instr_retire(const MonInstrTxn* txn);
int mon_instr_retire_simple(uint32_t order, uint32_t pc, uint32_t instr, int trap,
                            int gpr_valid, uint32_t rd_addr, uint32_t rd_wdata);
void mon_instr_finish();
void mon_instr_reset();

}

#endif
