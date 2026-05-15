## ADDED Requirements

### Requirement: Ibex shall emit monitor retire transactions
The system SHALL emit a project-local monitor retire transaction for each Ibex
retire event observed by the existing CPU-mode checker, while preserving the
current upstream-style cosim synchronization path as the authoritative checker.

#### Scenario: Ibex retire produces monitor log output
- **WHEN** an Ibex CPU-mode run retires an instruction with known RVFI retire fields
- **THEN** the run SHALL append a monitor transaction entry to an Ibex-specific monitor log

#### Scenario: Ibex checker semantics remain unchanged
- **WHEN** an Ibex CPU-mode run performs retire comparison, interrupt/debug synchronization, and dside access synchronization
- **THEN** those checks SHALL continue to use the existing checker path independently of the monitor log path

### Requirement: VeeR EL2 shall route retire events through the monitor runtime
The system SHALL translate VeeR EL2 CPU-mode retire observations into the
shared monitor transaction format and SHALL write them through the project-local
monitor runtime.

#### Scenario: VeeR trace retire produces monitor transaction
- **WHEN** a VeeR EL2 CPU-mode run observes a valid public trace retire event
- **THEN** the run SHALL create a monitor transaction containing the retire PC, instruction, and trap indication

#### Scenario: VeeR first-stage monitor leaves unavailable resources invalid
- **WHEN** a VeeR EL2 CPU-mode run does not yet expose architectural GPR or CSR retire resources
- **THEN** the monitor transaction SHALL mark those resources invalid rather than inventing values

### Requirement: Monitor logs shall be target-specific
The system SHALL allow each CPU target using the monitor runtime to select an
explicit monitor log path.

#### Scenario: Ibex and VeeR use separate monitor logs
- **WHEN** Ibex and VeeR EL2 CPU-mode runs are executed
- **THEN** each target SHALL write its monitor evidence to a dedicated default log path rather than reusing the PicoRV32 log name
