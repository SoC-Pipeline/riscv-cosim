## ADDED Requirements

### Requirement: CosimSession shall support ibex-style detailed step verification
The CosimSession class SHALL provide a `step_detail()` method variant that supports ibex verification needs including write_reg, sync_trap, suppress_reg_write parameters while maintaining backward compatibility with existing retire() interface.

#### Scenario: Simple mode (picorv32/veer_el2)
- **WHEN** `retire(pc, instr)` is called with simple mode enabled
- **THEN** system SHALL compare golden PC and instruction against DUT values
- **AND** step count SHALL be incremented by 1

#### Scenario: Detailed mode (ibex)
- **WHEN** `step_detail(write_reg, write_reg_data, pc, sync_trap, suppress_reg_write)` is called
- **THEN** system SHALL perform detailed verification including register write check
- **AND** system SHALL handle sync_trap (synchronous trap) cases
- **AND** system SHALL suppress reg_write when suppress_reg_write is true

### Requirement: CosimSession shall support CSR operations
The CosimSession class SHALL provide methods to access and modify Control Status Registers:
- `set_csr(unsigned csr_num, uint32_t value)`
- `get_csr(unsigned csr_num)`

#### Scenario: CSR write
- **WHEN** `set_csr(CSR_MSTATUS, value)` is called
- **THEN** the corresponding CSR in the reference simulator SHALL be updated

#### Scenario: CSR read
- **WHEN** `get_csr(CSR_MSTATUS)` is called
- **THEN** the current value of mstatus CSR SHALL be returned

### Requirement: CosimSession shall support interrupt and debug requests
The CosimSession class SHALL provide methods to set interrupt and debug states:
- `set_mip(uint32_t pre_mip, uint32_t post_mip)`
- `set_nmi(bool nmi)`
- `set_debug_req(bool debug_req)`

#### Scenario: NMI setting
- **WHEN** `set_nmi(true)` is called before step
- **THEN** the next step SHALL observe NMI state as pending

### Requirement: CosimSession shall support memory access notification
The CosimSession class SHALL provide `notify_dside_access()` method to track DUT memory transactions for ibex memory verification.

#### Scenario: Memory store notification
- **WHEN** DUT performs a store operation at address A with data D
- **THEN** `notify_dside_access(store=true, addr=A, data=D)` SHALL be called
- **AND** system SHALL compare against reference memory state