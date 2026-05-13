## ADDED Requirements

### Requirement: Ibex Verilator cosim target
The system SHALL provide an Ibex simulation target that runs the Ibex
simple-system Verilator co-simulation environment.

#### Scenario: Ibex target is selected
- **WHEN** a user runs `./build.sh sim ibex`
- **THEN** the build runs the Ibex Verilator simple-system cosim flow
- **AND** the existing PicoRV32 Icarus simulation is not invoked

### Requirement: Ibex firmware image
The system SHALL build an Ibex-compatible firmware image from project firmware
inputs using the Ibex simple-system memory map and halt convention.

#### Scenario: Ibex firmware is generated
- **WHEN** the Ibex simulation target prepares firmware
- **THEN** it produces an ELF linked for the Ibex simple-system RAM region
- **AND** the firmware can halt the Ibex simple-system simulation through its
  simulator-control device

### Requirement: Ibex cosim result is checked
The Ibex simulation target SHALL fail the build when the Ibex cosim checker
reports a mismatch or the Verilator simulation fails.

#### Scenario: Ibex cosim passes
- **WHEN** the Ibex simulation completes without checker mismatches
- **THEN** the command exits successfully
- **AND** the output reports that co-simulation matched instructions

#### Scenario: Ibex cosim fails
- **WHEN** the Ibex cosim checker reports a mismatch
- **THEN** the command exits with a non-zero status

### Requirement: Ibex dependency validation
The Ibex simulation target SHALL validate the external tools and Spike
pkg-config metadata required by the upstream Ibex cosim environment.

#### Scenario: Spike pkg-config metadata is missing
- **WHEN** `riscv-riscv`, `riscv-disasm`, or `riscv-fdt` cannot be found through
  pkg-config for the Ibex target
- **THEN** the build fails before running Verilator
- **AND** the error message identifies the missing Ibex cosim dependency
