#ifndef MON_MON_INSTR_TXN_INSTR_H
#define MON_MON_INSTR_TXN_INSTR_H

#include <cstdint>

struct MonInstrGpr {
    bool valid = false;
    uint32_t addr = 0;
    uint32_t data = 0;
};

struct MonInstrMem {
    bool valid = false;
    uint32_t addr = 0;
    uint32_t rmask = 0;
    uint32_t wmask = 0;
    uint32_t rdata = 0;
    uint32_t wdata = 0;
};

struct MonInstrCsr {
    bool valid = false;
    uint32_t addr = 0;
    uint64_t rmask = 0;
    uint64_t rdata = 0;
    uint64_t wmask = 0;
    uint64_t wdata = 0;
};

struct MonInstrTxn {
    uint32_t order = 0;
    uint32_t pc = 0;
    uint32_t instr = 0;
    bool trap = false;
    MonInstrGpr gpr;
    MonInstrMem mem;
    MonInstrCsr csr;
};

#endif
