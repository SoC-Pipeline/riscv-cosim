## ADDED Requirements

### Requirement: Default Spike commit logs for active cosim targets
The build and runtime flow SHALL enable Spike commit logging by default for the
active PicoRV32, Ibex, and VeeR EL2 cosim targets.

#### Scenario: PicoRV32 cosim runs with default settings
- **WHEN** a user runs `./build.sh run picorv32 hello`
- **THEN** the PicoRV32 cosim flow SHALL configure Spike with a non-empty
  commit log path
- **AND** Spike SHALL emit a commit log file for that run

#### Scenario: Ibex cosim runs with default settings
- **WHEN** a user runs `./build.sh run ibex hello`
- **THEN** the Ibex cosim flow SHALL configure Spike with a non-empty commit
  log path
- **AND** Spike SHALL emit a commit log file for that run

#### Scenario: VeeR EL2 cosim runs with default settings
- **WHEN** a user runs `./build.sh run veer_el2 hello`
- **THEN** the VeeR EL2 cosim flow SHALL configure Spike with a non-empty
  commit log path
- **AND** Spike SHALL emit a commit log file for that run

### Requirement: Aligned default firmware inputs for PicoRV32 cosim
The PicoRV32 cosim path SHALL use the active shared firmware layout for its
default fallback paths.

#### Scenario: PicoRV32 fallback firmware path is needed
- **WHEN** the PicoRV32 testbench or VPI bridge is started without an explicit
  firmware plusarg override
- **THEN** the default ELF path SHALL point to `build/firmware/hello/obj/firmware.elf`
- **AND** the default HEX path SHALL point to `build/firmware/hello/obj/firmware.hex`

### Requirement: Per-target default Spike commit log paths
Each active cosim target SHALL have a stable, target-specific default Spike
commit log path.

#### Scenario: Default log locations are used
- **WHEN** a user runs the default cosim flows without overriding log-related
  environment variables
- **THEN** PicoRV32, Ibex, and VeeR EL2 SHALL not overwrite each other's Spike
  commit logs
- **AND** the default log paths SHALL be documented in the build and
  architecture docs
