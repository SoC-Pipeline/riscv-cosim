## ADDED Requirements

### Requirement: Legacy threaded cosim path is removed from active source tree
The project SHALL remove unused legacy threaded cosim bridge files from
`src/cosim` to keep one active bridge/session/simulator implementation path.

#### Scenario: Developer checks active cosim source files
- **WHEN** a developer inspects `src/cosim` after this change
- **THEN** `spike_dpi_thread.cc`, `sim.h`, and `htif.h` SHALL not exist
- **AND** active cosim entry points SHALL continue to build through
  `spike_dpi.cc`, `cosim_bridge.cc`, `cosim_session.cc`, and
  `spike_simulator.cc`

### Requirement: Cosim configuration is built through a shared cpu-name policy
PicoRV32, Ibex, and VeeR EL2 cosim wrappers SHALL construct `CosimConfig`
through one shared helper keyed by `cpu_name`.

#### Scenario: Wrappers initialize cosim for different targets
- **WHEN** `spike_dpi.cc`, `tb_ibex_cosim.cc`, and `tb_veer_el2_cosim.cc`
  initialize cosim
- **THEN** each wrapper SHALL call the same shared config-policy helper
- **AND** wrapper-local duplicated log/default assembly SHALL be removed

### Requirement: Cosim log path policy is unified with compatibility overrides
Cosim compare-log and Spike commit-log paths SHALL use one precedence policy
across active targets while keeping existing target-specific environment
variables valid.

#### Scenario: User does not set log override variables
- **WHEN** a user runs an active cosim target without log overrides
- **THEN** compare log path SHALL default to
  `dump/<cpu_name>_cosim_result.log`
- **AND** Spike commit log path SHALL default to
  `dump/<cpu_name>_spike_commit.log`

#### Scenario: User sets generic override variables
- **WHEN** `COSIM_LOG_PATH` and/or `SPIKE_COMMIT_LOG_PATH` are set
- **THEN** active cosim wrappers SHALL use those generic values
- **AND** they SHALL take precedence over target-specific variables

#### Scenario: User uses existing target-specific variables
- **WHEN** generic override variables are unset and target-specific log
  variables are set
- **THEN** active wrappers SHALL honor the existing target-specific variables
  for compatibility

### Requirement: Unified policy is documented in build and architecture docs
The repository SHALL document the unified logging policy and its precedence in
both build usage help and architecture documentation.

#### Scenario: Developer reads build and architecture docs
- **WHEN** a developer checks `./build.sh` help text and `docs/arch.md`
- **THEN** both documents SHALL describe generic overrides, target-specific
  compatibility, and `cpu_name`-based default log naming
