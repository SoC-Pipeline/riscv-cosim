## 1. Phase 1: SpikeSimulator CSR Extension

- [x] 1.1 Add CSR method signatures to `RiscvSimulator` base class (riscv_simulator.h)
  - `virtual void set_csr(unsigned csr_num, uint32_t value) = 0`
  - `virtual uint32_t get_csr(unsigned csr_num) = 0`
  - `virtual void set_mip(uint32_t mip) = 0`
  - `virtual void set_nmi(bool nmi) = 0`
  - `virtual void set_debug_req(bool debug_req) = 0`

- [x] 1.2 Implement CSR methods in `SpikeSimulator::Impl` (spike_simulator.cc)
  - Use `processor_t::set_csr()` and `get_csr()` methods
  - Use `processor_t::set_mip()` for interrupt pending
  - Track NMI state and debug request state

- [x] 1.3 Verify picorv32 regression: `./build.sh run picorv32 hello`

- [x] 1.4 Verify veer_el2 regression: `./build.sh run veer_el2 hello`

## 2. Phase 2: CosimSession Interface Extension

- [x] 2.1 Add `step_detail()` method signature to CosimSession (cosim_session.h)
  - Signature: `void step_detail(uint32_t write_reg, uint32_t write_reg_data, uint32_t pc, bool sync_trap, bool suppress_reg_write)`

- [x] 2.2 Implement minimal `step_detail()` in cosim_session.cc
  - Initially delegate to existing `retire()` for backward compatibility
  - Add stub implementations for new interface methods

- [x] 2.3 Add `set_csr()`, `get_csr()`, `set_mip()`, `set_nmi()`, `set_debug_req()` to CosimSession
  - Delegate to `simulator_->` methods

- [x] 2.4 Add `notify_dside_access()` and error tracking methods
  - Add `errors_` vector and `clear_errors()`
  - Add `get_errors()` returning const reference

- [x] 2.5 Verify picorv32 regression still passes

- [x] 2.6 Verify veer_el2 regression still passes

## 3. Phase 3: tb_ibex_cosim Migration

- [x] 3.1 Update `tb_ibex_cosim.cc` to use CosimSession instead of SpikeCosim
  - Replace `#include "spike_cosim.h"` with `#include "cosim_session.h"`
  - Replace `std::unique_ptr<SpikeCosim>` with `std::unique_ptr<CosimSession>`
  - Update `CreateCosim()` to initialize CosimSession

- [x] 3.2 Update DPI entry points
  - `get_spike_cosim()` returns `Cosim*` - needs interface adaptation
  - Consider wrapping CosimSession in adapter that exposes Cosim interface

- [x] 3.3 Update ibex cosim build to link against CosimSession

- [x] 3.4 Verify ibex regression: `./build.sh run ibex hello`

## 4. Phase 4: Verification Logic Porting

- [x] 4.1 Port `check_retired_instr()` logic from SpikeCosim
  - Compare instruction at DUT PC vs reference
  - Handle sync_trap correctly

- [x] 4.2 Port `check_gpr_write()` logic
  - Verify write_reg and write_reg_data match reference state

- [x] 4.3 Port `handle_csr_ops()` logic
  - Track mcycle, mip, and other CSRs

- [x] 4.4 Enable full ibex verification mode in CosimSession

- [x] 4.5 Run all-regression comparison: `./build.sh run all all`
  - Compare commit logs across picorv32, ibex, veer_el2
  - Use `scripts/compare.py` to verify identical output

## 5. Cleanup

- [x] 5.1 Remove or deprecate SpikeCosim usage in tb_ibex_cosim.cc
- [x] 5.2 Document new CosimSession interface
- [x] 5.3 Final regression test all targets
