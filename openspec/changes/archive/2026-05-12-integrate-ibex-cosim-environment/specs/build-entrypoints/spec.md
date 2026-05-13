## ADDED Requirements

### Requirement: Simulation target selection
The build script SHALL allow users to select the simulation target while
preserving the current PicoRV32 default behavior.

#### Scenario: Default simulation target
- **WHEN** a user runs `./build.sh sim`
- **THEN** the build runs the existing PicoRV32 simulation flow

#### Scenario: PicoRV32 target is selected explicitly
- **WHEN** a user runs `./build.sh sim picorv32`
- **THEN** the build runs the existing PicoRV32 simulation flow

#### Scenario: Ibex target is selected explicitly
- **WHEN** a user runs `./build.sh sim ibex`
- **THEN** the build runs the Ibex Verilator cosim flow

### Requirement: Simulation target documentation
The project documentation SHALL describe the available simulation targets and
the Ibex-specific tool requirements.

#### Scenario: User reads architecture documentation
- **WHEN** a user reads `docs/arch.md`
- **THEN** the documentation lists both PicoRV32 and Ibex simulation targets
- **AND** it explains the Ibex Verilator/FuseSoC and Spike pkg-config
  requirements
