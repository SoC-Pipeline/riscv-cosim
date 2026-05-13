## ADDED Requirements

### Requirement: Neutral cosim task API

The active VPI module SHALL expose simulator-neutral `$cosim_*` tasks for
design-owned testbenches.

#### Scenario: Testbench initializes cosim

- **WHEN** a testbench calls `$cosim_init` with an ELF path
- **THEN** the VPI runtime initializes the default reference simulator backend
- **AND** the testbench does not need to call a Spike-specific task

#### Scenario: Testbench reports DUT retire events

- **WHEN** a testbench calls `$cosim_retire` with DUT PC and instruction values
- **THEN** the VPI runtime compares that event against the current reference
  simulator PC and instruction
- **AND** advances the reference simulator for the next retire event when
  appropriate

### Requirement: Compare state is owned by cosim runtime

The active co-simulation implementation SHALL keep compare counters and log
generation inside the C++ VPI runtime.

#### Scenario: Testbench finishes simulation

- **WHEN** a testbench calls `$cosim_finish`
- **THEN** the VPI runtime prints compare pass and fail counts
- **AND** writes the compare log
- **AND** the testbench does not need to maintain golden PC/instruction arrays

### Requirement: PicoRV32 remains an example integration

The current PicoRV32 simulation SHALL continue to run as an example design
testbench integration using the neutral cosim task API.

#### Scenario: Existing regression runs

- **WHEN** `./build.sh all` runs the PicoRV32 regression
- **THEN** the testbench uses `$cosim_init`, `$cosim_retire`, and
  `$cosim_finish`
- **AND** the regression completes with zero compare failures

### Requirement: Spike task compatibility is removed

The active VPI module SHALL not preserve the old `$spike_*` task API.

#### Scenario: VPI tasks are registered

- **WHEN** the active VPI module registers system tasks
- **THEN** it registers `$cosim_*` tasks
- **AND** it does not register `$spike_*` compatibility tasks
