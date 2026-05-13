## ADDED Requirements

### Requirement: picorv32 shall retain VPI bridge support during transition
The project SHALL preserve the existing picorv32 VPI path as a supported baseline while evaluating DPI integration.

#### Scenario: baseline run
- **WHEN** picorv32 is run in baseline mode
- **THEN** the VPI bridge path SHALL remain functional and regression-capable

### Requirement: picorv32 shall provide a DPI prototype mode
The project SHALL provide a selectable DPI bridge path for picorv32 so maintainability and behavior can be evaluated against VPI.

#### Scenario: explicit mode select
- **WHEN** user selects picorv32 DPI mode
- **THEN** co-sim SHALL execute through the DPI bridge path
- **AND** lifecycle semantics SHALL match the canonical bridge contract

### Requirement: fallback policy shall be explicit
If picorv32 DPI path has unresolved issues, both VPI and DPI modes SHALL be kept and documented with recommended usage.

#### Scenario: DPI instability found
- **WHEN** DPI mode fails acceptance criteria
- **THEN** VPI mode SHALL remain supported
- **AND** documentation SHALL describe limitations and fallback guidance
