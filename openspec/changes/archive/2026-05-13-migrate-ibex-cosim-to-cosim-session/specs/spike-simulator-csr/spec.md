## ADDED Requirements

### Requirement: SpikeSimulator shall expose CSR read/write operations
The SpikeSimulator class SHALL provide methods to read and write Control Status Registers through the underlying processor_t instance.

#### Scenario: CSR write
- **WHEN** `set_csr(unsigned csr_num, uint32_t value)` is called
- **THEN** the Spike processor SHALL update the corresponding CSR
- **AND** subsequent CSR reads SHALL reflect the new value

#### Scenario: CSR read
- **WHEN** `get_csr(unsigned csr_num)` is called
- **THEN** the current value of the specified CSR SHALL be returned

### Requirement: SpikeSimulator shall support MIP modification
The SpikeSimulator class SHALL provide `set_mip()` method to update the Machine Interrupt Pending register.

#### Scenario: Set pending interrupt
- **WHEN** `set_mip(0x00000080)` is called (external interrupt pending)
- **THEN** the processor MIP register SHALL be updated
- **AND** subsequent step SHALL check for interrupt handling

### Requirement: SpikeSimulator shall support NMI and debug requests
The SpikeSimulator class SHALL provide methods to set Non-Maskable Interrupt and debug request states.

#### Scenario: NMI assertion
- **WHEN** `set_nmi(true)` is called
- **THEN** NMI SHALL be pending for next step
- **AND** after mret, new NMI SHALL be taken if NMI is still high

#### Scenario: Debug request
- **WHEN** `set_debug_req(true)` is called
- **THEN** debug mode entry SHALL be pending for next step