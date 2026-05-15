## ADDED Requirements

### Requirement: PicoRV32 RVFI monitor integration
The system SHALL integrate PicoRV32 CPU-mode co-simulation through the RVFI retire interface exposed by the PicoRV32 wrapper.

#### Scenario: PicoRV32 retire packet is reported
- **WHEN** `./build.sh run cpu picorv32 hello` runs and PicoRV32 retires an instruction with known RVFI fields
- **THEN** the co-simulation path receives a retire packet containing at least PC, instruction, trap state, GPR writeback address, and GPR writeback data

#### Scenario: Third-party PicoRV32 RTL remains unmodified
- **WHEN** PicoRV32 monitor support is added
- **THEN** the implementation does not require source edits in `external/picorv32/picorv32.v`

### Requirement: Detail co-simulation comparison
The system SHALL compare PicoRV32 retired PC and GPR writeback data against Spike in CPU mode.

#### Scenario: Matching GPR writeback passes
- **WHEN** PicoRV32 retires an instruction whose PC and GPR writeback match Spike
- **THEN** the co-simulation result records the instruction as passing

#### Scenario: Mismatched GPR writeback fails
- **WHEN** PicoRV32 retires an instruction whose reported GPR writeback register or data differs from Spike
- **THEN** the co-simulation result records a failure and exposes the mismatch through existing run status reporting

### Requirement: Monitor resource transaction structure
The system SHALL represent retire monitor data as resource observations that can carry GPR and CSR state while preserving the existing PicoRV32 RVFI retire flow.

#### Scenario: GPR resource remains comparable
- **WHEN** PicoRV32 retires an instruction with a known GPR writeback
- **THEN** the monitor transaction records the GPR as a valid resource observation and the co-simulation path compares it against Spike

#### Scenario: Unsupported FPR and vector resources are not claimed
- **WHEN** the PicoRV32 monitor path is used
- **THEN** FPR and vector register comparison is not reported as supported for PicoRV32

### Requirement: PicoRV32 CSR monitor logging and policy
The system SHALL capture PicoRV32 RVFI CSR read observations for supported CSR signals and apply a documented comparison policy.

#### Scenario: Counter CSR read is logged
- **WHEN** PicoRV32 retires an instruction with RVFI `mcycle` or `minstret` CSR read data
- **THEN** the PicoRV32 monitor log records the CSR address, read mask, and read data

#### Scenario: Volatile counter CSR reads are not mandatory compare points
- **WHEN** PicoRV32 reports `cycle`, `cycleh`, `instret`, or `instreth` CSR reads
- **THEN** the monitor logs the observation and does not fail solely because Spike's volatile counter value differs

#### Scenario: Stable CSR read mismatch fails
- **WHEN** a supported non-volatile CSR read observation is compared and the masked DUT value differs from Spike
- **THEN** the co-simulation result records a failure and exposes the mismatch through existing run status reporting

### Requirement: PicoRV32 monitor logging
The system SHALL write PicoRV32 monitor events to a dedicated monitor log file.

#### Scenario: Monitor log is created
- **WHEN** `./build.sh run cpu picorv32 hello` runs
- **THEN** a PicoRV32 monitor log file is created under the configured log location

#### Scenario: Monitor log records retire details
- **WHEN** PicoRV32 retires instructions during a CPU-mode run
- **THEN** the monitor log contains per-retire details including order or cycle, PC, instruction, trap state, GPR writeback information, memory fields, and CSR observations when present

### Requirement: Existing CPU run flow remains compatible
The system SHALL preserve the existing PicoRV32 CPU build and run commands while adding monitor-based checks.

#### Scenario: PicoRV32 hello run completes
- **WHEN** `./build.sh run cpu picorv32 hello` is executed
- **THEN** the command builds required firmware and simulation artifacts and completes using the monitor-based co-simulation path

#### Scenario: All CPU runs remain available
- **WHEN** `./build.sh run all all` is executed
- **THEN** the PicoRV32 CPU run remains part of the normal all-target flow

### Requirement: Architecture documentation is updated
The system SHALL document the CPU-mode monitor data flow and the check coverage introduced for PicoRV32.

#### Scenario: CPU monitor architecture is documented
- **WHEN** a developer reads `docs/arch.md`
- **THEN** the document explains that PicoRV32 RVFI retire data feeds a monitor layer and that the current Spike comparison covers retired PC plus GPR writeback, with CSR policy and FPR/VR limitations documented
