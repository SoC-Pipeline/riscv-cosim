## ADDED Requirements

### Requirement: Active generated artifacts live under build

The repository SHALL place active co-simulation generated artifacts under `build/` instead of writing them into source/input directories.

#### Scenario: Regression builds firmware

- **WHEN** `make all` runs for the default test
- **THEN** firmware object files, ELF, HEX, BIN, and LST outputs are generated under `build/firmware/arith_basic_test/obj/`
- **AND** the build does not regenerate those active outputs under `firmware/arith_basic_test/obj/`

#### Scenario: Regression builds source-side generated outputs

- **WHEN** `make all` runs
- **THEN** the generated Icarus simulation binary is written under `build/src/top/`
- **AND** Spike VPI module artifacts are written under `build/src/cosim/`
- **AND** the active build does not regenerate `src/top/testbench.vvp` or `scripts/libspike.vpi`

### Requirement: Active regression consumes build artifacts consistently

The repository SHALL pass generated firmware and VPI artifact paths from `build/` to the active co-simulation tools.

#### Scenario: Simulation launches after build

- **WHEN** `make all` reaches the simulation step
- **THEN** `vvp` loads the Spike VPI module from `build/src/cosim/`
- **AND** the Verilog firmware plusarg points at `build/firmware/<TEST_NAME>/obj/firmware.hex`
- **AND** the Spike ELF plusarg points at `build/firmware/<TEST_NAME>/obj/firmware.elf`

#### Scenario: Plusargs are omitted in a direct testbench run

- **WHEN** the Verilog testbench runs without explicit `ELF_PATH` or `firmware` plusargs
- **THEN** its default fallback paths refer to `build/firmware/arith_basic_test/obj/firmware.elf` and `build/firmware/arith_basic_test/obj/firmware.hex`

### Requirement: Cleanup removes generated build outputs

The repository SHALL provide cleanup for active generated outputs under `build/`.

#### Scenario: Developer runs clean

- **WHEN** `make clean` runs
- **THEN** active generated artifacts under `build/` and simulation logs under `dump/` are removed
- **AND** source/input directories remain intact

### Requirement: Documentation describes generated output layout

The repository SHALL document `build/` as the generated-output location for active co-simulation artifacts.

#### Scenario: Developer reads project documentation

- **WHEN** a developer opens `README.md` or `docs/arch.md`
- **THEN** the documentation identifies `build/firmware` and `build/src` as generated-output areas for the active regression
