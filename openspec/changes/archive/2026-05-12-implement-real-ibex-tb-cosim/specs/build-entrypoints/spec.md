## ADDED Requirements

### Requirement: Ibex build entrypoint uses project top
The build script SHALL build and run the project-owned `tb_ibex` cosim target
when the Ibex simulation target is selected.

#### Scenario: Ibex target is selected
- **WHEN** a user runs `./build.sh sim ibex`
- **THEN** the build runs the project `tb_ibex` Verilator cosim flow
- **AND** the PicoRV32 Icarus/VPI simulation is not invoked

### Requirement: PicoRV32 default remains unchanged
The build script SHALL preserve the PicoRV32 simulation as the default
simulation target.

#### Scenario: Default simulation target
- **WHEN** a user runs `./build.sh sim`
- **THEN** the build runs the existing PicoRV32 simulation flow
