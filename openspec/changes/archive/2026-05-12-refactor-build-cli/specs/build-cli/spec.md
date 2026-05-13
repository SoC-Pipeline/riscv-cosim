## ADDED Requirements

### Requirement: Split Build And Run Commands

`build.sh` SHALL expose explicit build and run commands for dependency,
firmware, top, and execution flows.

#### Scenario: Build Spike Dependencies

- **WHEN** a user runs `./build.sh build spike`
- **THEN** the script builds local Spike into `build/spike`
- **AND** builds local pk into `build/pk`
- **AND** builds the PicoRV32 VPI module under `build/src/cosim`.
- **AND** the default Spike source provides the Ibex cosim API required by the
  Ibex DPI checker.

#### Scenario: Build Firmware Cases

- **WHEN** a user runs `./build.sh build -f all`
- **THEN** the script builds every case listed by `FIRMWARE_CASES`.

#### Scenario: Build Simulation Tops

- **WHEN** a user runs `./build.sh build -t all`
- **THEN** the script builds both `picorv32` and `ibex` simulation top artifacts.

#### Scenario: Run Simulation Matrix

- **WHEN** a user runs `./build.sh run all all`
- **THEN** the script runs every active firmware case on both `picorv32` and
  `ibex`.

### Requirement: Remove Sim Command

`build.sh` SHALL NOT provide the old `sim` command.

#### Scenario: Sim Is Not Supported

- **WHEN** a user runs `./build.sh sim`
- **THEN** the command is rejected as an unknown command.

### Requirement: Preserve Slow Dependencies On Clean

`build.sh clean` SHALL preserve local Spike and pk build/install directories.

#### Scenario: Development Clean Preserves Spike

- **WHEN** a user runs `./build.sh clean`
- **THEN** `build/spike`, `build/spike-build`, `build/pk`, and `build/pk-build`
  are not removed.
- **AND** generated firmware, top, cosim, cache, and dump outputs are removed.

#### Scenario: Full Clean Removes Build Directory

- **WHEN** a user runs `./build.sh clean-all`
- **THEN** the whole `build/` directory is removed.
- **AND** `dump/` is removed.

### Requirement: Ibex Uses Local Spike

The Ibex cosim build SHALL use the local `build/spike` installation by default.

#### Scenario: Ibex Pkg Config Uses Build Spike

- **WHEN** the Ibex top is built through `./build.sh build -t ibex`
- **THEN** `PKG_CONFIG_PATH` points at `build/spike/lib/pkgconfig` by default.
- **AND** the build does not require `external/ibex/build/spike`.
