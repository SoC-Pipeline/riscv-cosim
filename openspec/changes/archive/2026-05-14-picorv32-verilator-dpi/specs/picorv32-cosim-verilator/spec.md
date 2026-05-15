## ADDED Requirements

### Requirement: PicoRV32 CPU-mode SHALL run as a Verilator top-level testbench
The system SHALL build and run PicoRV32 CPU-mode simulation with `tb_picorv32` as the simulation top under Verilator, without requiring `iverilog`, `vvp`, or a VPI module at runtime.

#### Scenario: Building PicoRV32 CPU-mode simulation
- **WHEN** the PicoRV32 CPU-mode top is built
- **THEN** the build system SHALL generate a Verilator executable for `tb_picorv32`
- **AND** the build SHALL compile the testbench with timing support required by its behavioral delays

#### Scenario: Running PicoRV32 CPU-mode simulation
- **WHEN** a PicoRV32 firmware case is simulated
- **THEN** the simulator SHALL run as a native Verilator executable
- **AND** the simulation SHALL accept the same firmware and ELF inputs required for co-simulation

### Requirement: PicoRV32 CPU-mode SHALL use DPI for co-simulation callbacks
The system SHALL use DPI-C callbacks for PicoRV32 co-simulation initialization, retire reporting, and finish handling, and SHALL not depend on PicoRV32-specific `$cosim_*` VPI system tasks.

#### Scenario: Initializing co-simulation
- **WHEN** `tb_picorv32` releases reset and starts co-simulation
- **THEN** the testbench SHALL call a DPI-C initialization entrypoint with the selected ELF path

#### Scenario: Reporting instruction retirement
- **WHEN** `tb_picorv32` observes a valid, known RVFI retirement event
- **THEN** the testbench SHALL forward the full RVFI-derived retire payload through a DPI-C callback

#### Scenario: Finishing co-simulation
- **WHEN** the testbench ends the simulation because of PASS, FAIL, trap, or timeout handling
- **THEN** the testbench SHALL invoke a DPI-C finish entrypoint exactly once before terminating

### Requirement: PicoRV32 monitor fidelity SHALL be preserved after the DPI migration
The system SHALL preserve PicoRV32's current retire-monitor fidelity by reconstructing a `MonInstrTxn` equivalent from the DPI retire payload, including instruction, GPR, memory, and CSR information.

#### Scenario: GPR and memory retirement data
- **WHEN** PicoRV32 retires an instruction with register or memory side effects
- **THEN** the DPI-backed monitor path SHALL preserve the retired register write and memory access fields passed to `mon_instr`

#### Scenario: CSR retirement data
- **WHEN** PicoRV32 retires an instruction with supported CSR activity
- **THEN** the DPI-backed monitor path SHALL preserve the CSR address, masks, and data fields passed to `mon_instr`
