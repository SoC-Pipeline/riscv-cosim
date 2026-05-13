## ADDED Requirements

### Requirement: Repository layout separates source, firmware, scripts, and external dependencies
The repository SHALL place project-owned RTL and co-simulation bridge source under `src/`, firmware test inputs under `firmware/`, Python utilities under `scripts/`, and external dependency drops under `external/`.

#### Scenario: Developer inspects the repository root
- **WHEN** a developer lists the repository root after the migration
- **THEN** the root contains `src/`, `firmware/`, `scripts/`, `external/`, `README.md`, and `TODO.md`

#### Scenario: Python utilities remain scripts
- **WHEN** the co-simulation build invokes Python helper utilities
- **THEN** it uses the utilities from `scripts/`

### Requirement: Primary co-simulation regression remains path-correct
The repository SHALL keep `make all` as the primary regression entry point and SHALL pass the correct firmware ELF, firmware HEX, RTL files, and VPI bridge paths to the build and simulation tools.

#### Scenario: Regression builds firmware
- **WHEN** `make all` runs with the required RISC-V toolchain available
- **THEN** the firmware ELF and HEX are generated under the selected `firmware/<test-name>/obj/` directory

#### Scenario: Regression launches co-simulation
- **WHEN** `make all` reaches the simulation step
- **THEN** Icarus Verilog compiles RTL from `src/top/` and VPI loads the Spike bridge from the configured VPI module directory

### Requirement: Default testbench paths match migrated firmware layout
The Verilog testbench SHALL use the migrated firmware path for default ELF and HEX fallback values when plusargs are not provided.

#### Scenario: Plusargs are omitted
- **WHEN** the testbench runs without explicit `ELF_PATH` or `firmware` plusargs
- **THEN** the fallback paths refer to `firmware/arith_basic_test/obj/firmware.elf` and `firmware/arith_basic_test/obj/firmware.hex`

### Requirement: Architecture documentation describes the migrated layout
The repository SHALL include architecture documentation describing the main directory roles, build flow, and expected regression command.

#### Scenario: Developer reads architecture documentation
- **WHEN** a developer opens `docs/arch.md`
- **THEN** it explains the roles of `src/`, `firmware/`, `scripts/`, `external/`, and the `make all` co-simulation flow
