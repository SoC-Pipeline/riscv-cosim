## ADDED Requirements

### Requirement: Default hello firmware
The build system SHALL use `hello` as the default firmware test case.

#### Scenario: Default firmware command is used
- **WHEN** a user runs `./build.sh firmware` without `TEST_NAME`
- **THEN** the build produces `build/firmware/hello/obj/firmware.elf`
- **AND** the build produces `build/firmware/hello/obj/firmware.hex`

### Requirement: Self-contained hello firmware
The repository SHALL provide a self-contained `firmware/hello` case.

#### Scenario: Hello firmware is built
- **WHEN** the hello firmware is compiled
- **THEN** all required hello startup, linker, print, and C sources come from
  `firmware/hello`
- **AND** the firmware prints `hello world`
- **AND** the firmware terminates through the shared finish MMIO address

### Requirement: Shared print MMIO
PicoRV32 and Ibex testbenches SHALL both support character output written to
`0x10000000`.

#### Scenario: Firmware prints characters
- **WHEN** firmware writes byte values to `0x10000000`
- **THEN** the active testbench emits those bytes to simulation output as a
  character stream

### Requirement: Existing arithmetic case remains selectable
The existing arithmetic firmware case SHALL remain buildable by explicit
selection.

#### Scenario: Arithmetic firmware is selected
- **WHEN** a user runs `TEST_NAME=arith_basic_test ./build.sh firmware`
- **THEN** the existing arithmetic firmware is built under
  `build/firmware/arith_basic_test/obj`
