## ADDED Requirements

### Requirement: Spike is built from local source

The repository SHALL build Spike from the local `external/riscv-isa-sim` source directory into `build/spike`.

#### Scenario: Developer builds Spike dependency

- **WHEN** a developer runs `./build.sh spike`
- **THEN** Spike is configured and built from `external/riscv-isa-sim`
- **AND** installed under `build/spike`

#### Scenario: VPI bridge is built

- **WHEN** a developer runs `./build.sh vpi` after local Spike is installed
- **THEN** the VPI bridge uses Spike headers from `build/spike/include`
- **AND** links Spike libraries from `build/spike/lib`

### Requirement: pk is built from local source

The repository SHALL build pk from the local `external/riscv-pk` source directory into `build/pk`.

#### Scenario: Developer builds pk dependency

- **WHEN** a developer runs `./build.sh pk`
- **THEN** pk is configured and built from `external/riscv-pk`
- **AND** installed under `build/pk`

#### Scenario: Simulation runs

- **WHEN** `./build.sh all` reaches simulation
- **THEN** the `PK_PATH` plusarg points at `build/pk/riscv32-unknown-elf/bin/pk`
- **AND** the co-simulation completes with zero compare failures

### Requirement: Current regression builds dependencies locally

The active regression SHALL build and consume local Spike and pk dependencies by default.

#### Scenario: Developer runs current regression

- **WHEN** a developer runs `./build.sh clean && ./build.sh all` with required tool modules loaded
- **THEN** firmware, Spike, pk, VPI, and simulation artifacts are generated under `build/`
- **AND** the co-simulation completes with zero compare failures

### Requirement: Dependency metadata is documented

The repository SHALL document the local Spike and pk dependency sources and generated install locations.

#### Scenario: Developer inspects metadata and docs

- **WHEN** a developer opens `.gitmodules`, `README.md`, `docs/arch.md`, or `TODO.md`
- **THEN** local Spike and pk dependencies and their `build/` install locations are described
