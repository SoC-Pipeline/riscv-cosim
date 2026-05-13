## ADDED Requirements

### Requirement: Common firmware runtime
Active firmware cases SHALL use shared runtime support from `firmware/common`.

#### Scenario: Firmware case is built
- **WHEN** `./build.sh firmware` builds an active case
- **THEN** common startup, linker, and print sources are used
- **AND** the selected case only provides case-specific program code

### Requirement: Pico test runs in cosim
`firmware/pico_test` SHALL run in the active shared PicoRV32/Ibex cosim address
map.

#### Scenario: Pico test is selected
- **WHEN** `TEST_NAME=pico_test ./build.sh sim` runs
- **THEN** firmware starts at `0x80000080`
- **AND** the test prints its arithmetic output
- **AND** co-simulation completes without compare failures

### Requirement: Active firmware cases
The active firmware case set SHALL be explicit and exclude `arith_basic_test`.

#### Scenario: Default case list is used
- **WHEN** `./build.sh all` runs without `FIRMWARE_CASES`
- **THEN** the active firmware cases are `hello` and `pico_test`
- **AND** `firmware/common` is not treated as a case
- **AND** `arith_basic_test` is not treated as a case

### Requirement: All command regression
`./build.sh all` SHALL run each active firmware case on both supported
testbenches.

#### Scenario: Regression is run
- **WHEN** a user runs `./build.sh all`
- **THEN** each active firmware case is run on PicoRV32
- **AND** each active firmware case is run on Ibex
- **AND** the command fails if any case/target fails
