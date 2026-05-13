## ADDED Requirements

### Requirement: A canonical co-sim bridge API shall exist for all Spike-backed targets
The project SHALL define one canonical bridge API surface for co-sim lifecycle operations used by all targets (`picorv32`, `ibex`, `veer_el2`), independent of frontend mechanism (VPI/DPI/C++ adapter).

#### Scenario: Unified lifecycle contract
- **WHEN** any target starts co-simulation
- **THEN** it SHALL use the canonical bridge lifecycle (`init`, `retire/step_detail`, `finish`, `destroy`)
- **AND** wrapper-specific entry names SHALL map to that canonical lifecycle

#### Scenario: Canonical operation set
- **WHEN** wrappers integrate with the bridge
- **THEN** the canonical API SHALL include init, retire, step_detail, finish, destroy/reset, status count query, error query, and optional state sync operations used by detailed checkers

### Requirement: Bridge wrappers shall be thin shims
VPI, DPI, and C++ adapter wrappers SHALL only translate calling conventions and SHALL NOT duplicate co-sim verification logic.

#### Scenario: No duplicated verification behavior
- **WHEN** a wrapper receives a retire/detail callback
- **THEN** it SHALL forward to canonical bridge API
- **AND** verification semantics SHALL remain implemented in `CosimSession`

#### Scenario: Legacy wrapper names remain stable
- **WHEN** migration to canonical bridge is complete
- **THEN** existing wrapper-facing names (`$cosim_*`, Ibex `Cosim` methods, VeeR DPI names) SHALL remain usable
- **AND** they SHALL behave as aliases to canonical bridge operations

### Requirement: Bridge finish and destroy operations shall be safe and idempotent
The canonical bridge SHALL tolerate repeated `finish` and `destroy` calls without undefined behavior.

#### Scenario: repeated finish
- **WHEN** `finish` is invoked multiple times by wrapper final hooks
- **THEN** the bridge SHALL preserve stable status and avoid duplicate teardown faults

#### Scenario: destroy after finish
- **WHEN** `destroy` is invoked after `finish`
- **THEN** resources SHALL be released safely without error

#### Scenario: wrapper shutdown hook cleanup
- **WHEN** simulator shutdown hooks call cleanup after earlier finish
- **THEN** repeated finish/reset sequences SHALL NOT crash or corrupt bridge state
