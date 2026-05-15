## ADDED Requirements

### Requirement: Top CPU DPI glue SHALL use a shared parent-directory helper
Top-level CPU DPI glue code SHALL use a shared helper to create the parent directory for monitor and related output log paths before opening those files.

#### Scenario: Custom monitor log path uses a new directory
- **WHEN** a CPU target is configured with a monitor log path whose parent directory does not exist
- **THEN** the shared helper creates that parent directory before monitor initialization opens the file

### Requirement: Top CPU monitor glue SHALL support retire-only monitor submission
The shared monitor interface SHALL provide a retire-only helper that records a retire packet using `order`, `pc`, `instr`, and `trap` without requiring the caller to manually populate unrelated `MonInstrTxn` fields.

#### Scenario: Retire-only target submits a monitor packet
- **WHEN** a CPU target has only retire-level architectural information available for a given instruction
- **THEN** the target can submit that retire observation through the shared retire-only helper

### Requirement: Target-specific runtime control SHALL remain target-owned
This refactor SHALL preserve target-specific simulator control flow, retire sampling, and compare authority boundaries.

#### Scenario: Ibex checker path remains authoritative
- **WHEN** the Ibex CPU target runs co-simulation
- **THEN** the upstream checker integration remains the authoritative compare path and the shared helper changes apply only to its local monitor glue
