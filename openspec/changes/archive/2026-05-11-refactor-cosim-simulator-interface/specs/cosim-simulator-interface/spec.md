## ADDED Requirements

### Requirement: Neutral simulator interface exists

The active co-simulation implementation SHALL expose a simulator-neutral C++ interface for the reference simulator operations used by the current scoreboard.

#### Scenario: VPI bridge initializes the reference simulator

- **WHEN** `$spike_init` is called by the existing testbench
- **THEN** the VPI bridge creates the default Spike-backed simulator through the neutral interface
- **AND** the Spike-specific lifecycle objects are not owned directly by the VPI task function

#### Scenario: VPI bridge advances the reference simulator

- **WHEN** `$spike_run_steps` is called
- **THEN** the VPI bridge forwards the request through the neutral simulator interface
- **AND** the current Spike backend advances by the requested instruction count

### Requirement: Compatibility VPI tasks remain stable

The refactor SHALL preserve the existing `$spike_*` VPI task names and observable behavior.

#### Scenario: Existing testbench runs

- **WHEN** the existing `src/top/testbench.v` calls `$spike_init`, `$spike_run_steps`, `$spike_get_pc`, and `$spike_get_instr`
- **THEN** no Verilog task name changes are required
- **AND** the co-simulation still completes with zero compare failures

### Requirement: Spike backend is isolated

Spike-specific API usage SHALL be isolated in a Spike adapter rather than spread across the VPI task implementation.

#### Scenario: Developer inspects active co-simulation code

- **WHEN** a developer opens the active VPI entrypoint
- **THEN** VPI argument conversion and task registration are separate from Spike object lifecycle and Spike state access
- **AND** future simulator backends can be added without duplicating the VPI task registration logic

### Requirement: Architecture documentation is updated

The repository SHALL document the active co-simulation split between VPI frontend, simulator interface, and Spike backend.

#### Scenario: Developer inspects architecture documentation

- **WHEN** a developer opens `docs/arch.md`
- **THEN** the active co-simulation flow describes the VPI/frontend/adapter relationship
- **AND** it states that Spike remains the only implemented backend in this change
