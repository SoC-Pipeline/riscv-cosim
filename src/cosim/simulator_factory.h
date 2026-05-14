#ifndef COSIM_SIMULATOR_FACTORY_H
#define COSIM_SIMULATOR_FACTORY_H

#include "riscv_simulator.h"

#include <memory>

std::unique_ptr<RiscvSimulator> create_simulator(const CosimConfig& config);

#endif
