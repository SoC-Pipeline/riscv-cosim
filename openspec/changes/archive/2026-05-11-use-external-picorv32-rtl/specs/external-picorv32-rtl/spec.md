## ADDED Requirements

### Requirement: Active simulation uses external PicoRV32 RTL
The active co-simulation build SHALL compile PicoRV32 from `external/picorv32/picorv32.v` instead of a project-owned copy under `src/top/`.

#### Scenario: Active regression is built
- **WHEN** `make all` invokes the Verilog simulation build
- **THEN** the Icarus command includes `external/picorv32/picorv32.v`
- **AND** it does not require `src/top/picorv32.v`

### Requirement: PicoRV32 dependency is recorded as a submodule
The repository SHALL record `external/picorv32` as a git submodule using the official upstream URL `git@github.com:YosysHQ/picorv32.git`.

#### Scenario: Dependency metadata is inspected
- **WHEN** `.gitmodules` is inspected
- **THEN** it contains a submodule entry for `external/picorv32`
- **AND** the URL is `git@github.com:YosysHQ/picorv32.git`

### Requirement: Duplicate project RTL copy is removed
The repository SHALL remove the tracked `src/top/picorv32.v` copy after the active build references the external RTL.

#### Scenario: Source tree is inspected
- **WHEN** the source tree is reviewed
- **THEN** `src/top/picorv32.v` is not present as a tracked project-owned RTL copy

### Requirement: Regression behavior is preserved
The change SHALL preserve the existing firmware build, VPI bridge build, simulation launch, and PicoRV32/Spike comparison behavior.

#### Scenario: Regression is run after migration
- **WHEN** `make clean && make all` runs with the required tool modules loaded
- **THEN** the co-simulation completes with zero compare failures
