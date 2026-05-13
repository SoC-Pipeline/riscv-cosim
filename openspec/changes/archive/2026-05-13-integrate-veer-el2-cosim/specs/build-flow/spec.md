## MODIFIED Requirements

### Requirement: Build script supports simulation targets
The build script SHALL support explicit build and run selection for each project simulation target.

#### Scenario: Run supported target
- **WHEN** a developer runs `./build.sh run <target> <case>` with `<target>` set to `picorv32`, `ibex`, or `veer_el2`
- **THEN** the build script builds missing dependencies and executes the selected firmware case on the selected target

#### Scenario: Reject unsupported target
- **WHEN** a developer runs `./build.sh run <target>` with an unsupported target name
- **THEN** the build script exits with an error and prints help text
