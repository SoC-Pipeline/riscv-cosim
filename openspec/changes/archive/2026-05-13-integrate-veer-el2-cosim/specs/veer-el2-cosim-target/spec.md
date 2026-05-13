## ADDED Requirements

### Requirement: VeeR EL2 target can build from project sources
The system SHALL provide a `veer_el2` top target that builds VeeR EL2 simulation artifacts through the project build script.

#### Scenario: Build VeeR EL2 top
- **WHEN** a developer runs `./build.sh build -t veer_el2`
- **THEN** generated VeeR EL2 configuration and simulator artifacts are written under `build/src/top/veer_el2`

### Requirement: VeeR EL2 target can run project firmware
The system SHALL run firmware cases from `firmware/` on VeeR EL2 using the shared firmware build output.

#### Scenario: Run default firmware on VeeR EL2
- **WHEN** a developer runs `./build.sh run veer_el2`
- **THEN** the `hello` firmware case is built if needed and executed on the VeeR EL2 target

#### Scenario: Run pico_test firmware on VeeR EL2
- **WHEN** a developer runs `./build.sh run veer_el2 pico_test`
- **THEN** the `pico_test` firmware case is built if needed and executed on the VeeR EL2 target

### Requirement: VeeR EL2 target uses shared reset vector
The system SHALL configure VeeR EL2 reset behavior from the project `RESET_VECTOR` setting.

#### Scenario: Override reset vector
- **WHEN** a developer exports `RESET_VECTOR` before building or running `veer_el2`
- **THEN** the VeeR EL2 configuration and firmware run use that reset vector consistently

### Requirement: VeeR EL2 target reports cosim status
The system SHALL compare VeeR EL2 retired instructions with Spike through the project cosim environment.

#### Scenario: Defer cosim until firmware run is stable
- **WHEN** Phase 1 VeeR EL2 firmware execution is still being brought up
- **THEN** the VeeR EL2 target may run without Spike cosim and MUST still report firmware pass/fail status through the project finish convention

#### Scenario: Successful cosim run
- **WHEN** VeeR EL2 executes a supported firmware case without retire mismatches
- **THEN** the run reports a cosim pass count and zero cosim failures before exiting successfully
