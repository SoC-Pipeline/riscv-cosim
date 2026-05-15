#ifndef MON_MON_INSTR_MON_INSTR_H
#define MON_MON_INSTR_MON_INSTR_H

#include "txn_instr.h"

#include <cstdint>
#include <fstream>
#include <string>

class MonInstr {
public:
    explicit MonInstr(std::string log_path);
    ~MonInstr();

    bool retire(const MonInstrTxn& txn);
    void finish();

private:
    void ensure_log_open();
    void log_retire(const MonInstrTxn& txn, int rc);

    std::string log_path_;
    std::ofstream log_;
    uint64_t retire_count_ = 0;
};

extern "C" {

void mon_instr_init(const char* log_path);
int mon_instr_retire(const MonInstrTxn* txn);
void mon_instr_finish();
void mon_instr_reset();

}

#endif
