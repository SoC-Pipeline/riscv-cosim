## ADDED Requirements

### Requirement: Project-owned Ibex cosim testbench
The system SHALL provide a real `tb_ibex` Verilator top under `src/top` that
instantiates Ibex, memory, bus, simulator control, timer, and the Ibex cosim
checker path.

#### Scenario: Ibex top is built
- **WHEN** the Ibex cosim simulation target is built
- **THEN** Verilator builds a `tb_ibex` top from project sources
- **AND** the upstream `ibex_simple_system` top is not the active simulation
  top

### Requirement: Shared firmware ELF
The Ibex cosim target SHALL use the same firmware ELF artifact as the PicoRV32
simulation target.

#### Scenario: Ibex simulation prepares firmware
- **WHEN** a user runs `./build.sh sim ibex`
- **THEN** the build generates `build/firmware/<TEST_NAME>/obj/firmware.elf`
- **AND** the Ibex simulator loads that ELF through its Verilator memory loader

### Requirement: Configurable Ibex reset vector
The Ibex cosim testbench SHALL make the effective reset vector configurable and
default it to the PicoRV32 firmware reset vector.

#### Scenario: Default reset vector is used
- **WHEN** the Ibex testbench is built without a reset-vector override
- **THEN** the Ibex cosim ISS starts at `0x80000080`
- **AND** the shared firmware ELF can start from its existing reset vector

#### Scenario: Reset vector is overridden
- **WHEN** the build passes an Ibex reset-vector override
- **THEN** the Ibex testbench and cosim harness use the overridden reset vector
  consistently

### Requirement: Ibex cosim result is checked
The Ibex cosim target SHALL fail the build when the Ibex cosim checker reports a
mismatch or the Verilator simulation fails.

#### Scenario: Ibex cosim passes
- **WHEN** the Ibex simulation completes without checker mismatches
- **THEN** the command exits successfully
- **AND** the output reports that co-simulation matched instructions
