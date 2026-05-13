## ADDED Requirements

### Requirement: Shell build entrypoint replaces Makefile

The repository SHALL use `build.sh` as the supported active build entry point instead of `Makefile`.

#### Scenario: Developer asks for build help

- **WHEN** a developer runs `./build.sh` with no arguments
- **THEN** the script prints command help
- **AND** exits successfully

#### Scenario: Developer runs the current regression

- **WHEN** a developer runs `./build.sh all` with required tool modules loaded
- **THEN** firmware artifacts are generated under `build/firmware/<TEST_NAME>/obj/`
- **AND** VPI artifacts are generated under `build/src/cosim/`
- **AND** the Icarus simulation binary is generated under `build/src/top/`
- **AND** the co-simulation completes with zero compare failures

#### Scenario: Developer cleans generated outputs

- **WHEN** a developer runs `./build.sh clean`
- **THEN** generated active outputs under `build/` and logs under `dump/` are removed
- **AND** source/input directories remain intact

#### Scenario: Makefile is no longer present

- **WHEN** the migration is complete
- **THEN** the repository root no longer contains `Makefile`

### Requirement: Build script preserves active override behavior

The build script SHALL preserve environment-variable overrides used by the active co-simulation flow.

#### Scenario: Developer overrides the default test

- **WHEN** a developer runs `TEST_NAME=<name> ./build.sh firmware`
- **THEN** the script reads firmware inputs from `firmware/<name>/`
- **AND** writes firmware outputs under `build/firmware/<name>/obj/`

#### Scenario: Developer overrides tool locations

- **WHEN** a developer provides toolchain, Spike, compiler, Icarus, or VVP override variables
- **THEN** `build.sh` uses those override values for the active commands

### Requirement: Documentation names build.sh commands

Project documentation SHALL describe `build.sh` as the supported command interface.

#### Scenario: Developer reads build documentation

- **WHEN** a developer opens `README.md`, `docs/arch.md`, or `TODO.md`
- **THEN** active build and test instructions use `./build.sh clean` and `./build.sh all` instead of Makefile commands
