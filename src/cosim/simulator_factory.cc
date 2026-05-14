#include "simulator_factory.h"

#include "spike_simulator.h"

#include <stdexcept>

std::unique_ptr<RiscvSimulator> create_simulator(const CosimConfig& config)
{
    if (config.backend.empty() || config.backend == "spike") {
        return std::make_unique<SpikeSimulator>();
    }

    throw std::runtime_error("unsupported simulator backend: " + config.backend);
}
